#ifdef CIRE_LLVM_FRONTEND_ENABLED

#include "llvm_frontend.hpp"

#include "graph/computation_graph.hpp"
#include "graph/node.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

namespace frontend::llvm_ir {

    // Helper function to load LLVM module from file
    static std::unique_ptr<llvm::Module> loadModule(llvm::LLVMContext& context, const std::filesystem::path& path) {
        llvm::SMDiagnostic err;
        auto module = llvm::parseIRFile(path.string(), err, context);

        if (!module) {
            std::string errMsg = "Failed to load LLVM IR file: " + path.string();
            if (err.getMessage().size() > 0) {
                errMsg += "\n" + err.getMessage().str();
            }
            throw std::runtime_error(errMsg);
        }

        return module;
    }

    // Helper function to find function by name
    static llvm::Function* findFunction(llvm::Module& module, const std::string& name) {
        auto* func = module.getFunction(name);
        if (!func || func->isDeclaration()) {
            return nullptr;
        }
        return func;
    }

    // Helper function to convert LLVM type to FloatPrec
    static graph::FloatPrec llvmTypeToPrec(llvm::Type* type) {
        if (type->isHalfTy()) {
            return graph::FloatPrec::F16;
        } else if (type->isFloatTy()) {
            return graph::FloatPrec::F32;
        } else if (type->isDoubleTy()) {
            return graph::FloatPrec::F64;
        } else {
            throw std::runtime_error("Unsupported LLVM floating-point type");
        }
    }

    // Helper class to manage the conversion context
    struct ConversionContext {
        graph::ComputationGraph& graph;
        std::map<llvm::Value*, graph::NodeId> valueMap;  // Map LLVM values to graph nodes

        explicit ConversionContext(graph::ComputationGraph& g) : graph(g) {}
    };

    // Get or create node for LLVM value
    static graph::NodeId getOrCreateNode(llvm::Value* val, ConversionContext& ctx) {
        // Check if we already have a node for this value
        auto it = ctx.valueMap.find(val);
        if (it != ctx.valueMap.end()) {
            return it->second;
        }

        // Handle constant values
        if (auto* fpConst = llvm::dyn_cast<llvm::ConstantFP>(val)) {
            double value = fpConst->getValueAPF().convertToDouble();
            graph::FloatPrec prec = llvmTypeToPrec(val->getType());

            graph::NodeId nodeId = ctx.graph.addNode(
                graph::ConstantNode{value, prec},
                prec
            );

            ctx.valueMap[val] = nodeId;
            return nodeId;
        }

        // Handle integer constants (convert to double)
        if (auto* intConst = llvm::dyn_cast<llvm::ConstantInt>(val)) {
            double value = static_cast<double>(intConst->getSExtValue());

            graph::NodeId nodeId = ctx.graph.addNode(
                graph::ConstantNode{value, graph::FloatPrec::F64},
                graph::FloatPrec::F64
            );

            ctx.valueMap[val] = nodeId;
            return nodeId;
        }

        throw std::runtime_error("Unable to find or create node for LLVM value");
    }

    // Parse function inputs
    static void parseInputs(llvm::Function& func, ConversionContext& ctx) {
        for (auto& arg : func.args()) {
            if (!arg.getType()->isFloatingPointTy()) {
                throw std::runtime_error("Only floating-point function arguments are supported");
            }

            std::string name = arg.getName().str();
            if (name.empty()) {
                name = "arg_" + std::to_string(arg.getArgNo());
            }

            graph::FloatPrec prec = llvmTypeToPrec(arg.getType());
            graph::NodeId inputId = ctx.graph.addInput(name, prec);
            ctx.valueMap[&arg] = inputId;
        }
    }

    // Parse function body (expressions)
    static void parseExpressions(llvm::Function& func, ConversionContext& ctx) {
        // Currently only support single basic block functions
        if (func.size() != 1) {
            throw std::runtime_error("Only single basic block functions are supported");
        }

        llvm::BasicBlock& bb = func.getEntryBlock();

        for (auto& inst : bb) {
            graph::NodeId nodeId;
            graph::FloatPrec prec;

            switch (inst.getOpcode()) {
                case llvm::Instruction::Ret: {
                    // Mark the return value as output
                    if (inst.getNumOperands() > 0) {
                        llvm::Value* retVal = inst.getOperand(0);
                        if (retVal->getType()->isFloatingPointTy()) {
                            auto it = ctx.valueMap.find(retVal);
                            if (it != ctx.valueMap.end()) {
                                ctx.graph.markOutput(it->second);
                            }
                        }
                    }
                    continue;
                }

                case llvm::Instruction::FNeg: {
                    if (!inst.getType()->isFloatingPointTy()) continue;

                    graph::NodeId op = getOrCreateNode(inst.getOperand(0), ctx);
                    prec = llvmTypeToPrec(inst.getType());
                    nodeId = ctx.graph.addNode(graph::NegNode{op}, prec);
                    break;
                }

                case llvm::Instruction::FAdd: {
                    if (!inst.getType()->isFloatingPointTy()) continue;

                    graph::NodeId lhs = getOrCreateNode(inst.getOperand(0), ctx);
                    graph::NodeId rhs = getOrCreateNode(inst.getOperand(1), ctx);
                    prec = llvmTypeToPrec(inst.getType());
                    nodeId = ctx.graph.addNode(graph::AddNode{lhs, rhs}, prec);
                    break;
                }

                case llvm::Instruction::FSub: {
                    if (!inst.getType()->isFloatingPointTy()) continue;

                    graph::NodeId lhs = getOrCreateNode(inst.getOperand(0), ctx);
                    graph::NodeId rhs = getOrCreateNode(inst.getOperand(1), ctx);
                    prec = llvmTypeToPrec(inst.getType());
                    nodeId = ctx.graph.addNode(graph::SubNode{lhs, rhs}, prec);
                    break;
                }

                case llvm::Instruction::FMul: {
                    if (!inst.getType()->isFloatingPointTy()) continue;

                    graph::NodeId lhs = getOrCreateNode(inst.getOperand(0), ctx);
                    graph::NodeId rhs = getOrCreateNode(inst.getOperand(1), ctx);
                    prec = llvmTypeToPrec(inst.getType());
                    nodeId = ctx.graph.addNode(graph::MulNode{lhs, rhs}, prec);
                    break;
                }

                case llvm::Instruction::FDiv: {
                    if (!inst.getType()->isFloatingPointTy()) continue;

                    graph::NodeId lhs = getOrCreateNode(inst.getOperand(0), ctx);
                    graph::NodeId rhs = getOrCreateNode(inst.getOperand(1), ctx);
                    prec = llvmTypeToPrec(inst.getType());
                    nodeId = ctx.graph.addNode(graph::DivNode{lhs, rhs}, prec);
                    break;
                }

                case llvm::Instruction::Call: {
                    auto* callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
                    if (!callInst || !callInst->getType()->isFloatingPointTy()) continue;

                    llvm::Function* calledFunc = callInst->getCalledFunction();
                    if (!calledFunc) {
                        throw std::runtime_error("Indirect calls not supported");
                    }

                    std::string funcName = calledFunc->getName().str();
                    prec = llvmTypeToPrec(inst.getType());

                    // Unary math functions
                    if (callInst->arg_size() == 1) {
                        graph::NodeId arg = getOrCreateNode(callInst->getArgOperand(0), ctx);

                        if (funcName == "sin" || funcName == "sinf" ||
                            funcName == "llvm.sin.f32" || funcName == "llvm.sin.f64") {
                            nodeId = ctx.graph.addNode(graph::SinNode{arg}, prec);
                        } else if (funcName == "cos" || funcName == "cosf" ||
                                   funcName == "llvm.cos.f32" || funcName == "llvm.cos.f64") {
                            nodeId = ctx.graph.addNode(graph::CosNode{arg}, prec);
                        } else if (funcName == "tan" || funcName == "tanf") {
                            nodeId = ctx.graph.addNode(graph::TanNode{arg}, prec);
                        } else if (funcName == "sqrt" || funcName == "sqrtf" ||
                                   funcName == "llvm.sqrt.f32" || funcName == "llvm.sqrt.f64") {
                            nodeId = ctx.graph.addNode(graph::SqrtNode{arg}, prec);
                        } else if (funcName == "exp" || funcName == "expf" ||
                                   funcName == "llvm.exp.f32" || funcName == "llvm.exp.f64") {
                            nodeId = ctx.graph.addNode(graph::ExpNode{arg}, prec);
                        } else if (funcName == "log" || funcName == "logf" ||
                                   funcName == "llvm.log.f32" || funcName == "llvm.log.f64") {
                            nodeId = ctx.graph.addNode(graph::LogNode{arg}, prec);
                        } else if (funcName == "asin" || funcName == "asinf") {
                            nodeId = ctx.graph.addNode(graph::AsinNode{arg}, prec);
                        } else if (funcName == "acos" || funcName == "acosf") {
                            nodeId = ctx.graph.addNode(graph::AcosNode{arg}, prec);
                        } else if (funcName == "atan" || funcName == "atanf") {
                            nodeId = ctx.graph.addNode(graph::AtanNode{arg}, prec);
                        } else if (funcName == "sinh" || funcName == "sinhf") {
                            nodeId = ctx.graph.addNode(graph::SinhNode{arg}, prec);
                        } else if (funcName == "cosh" || funcName == "coshf") {
                            nodeId = ctx.graph.addNode(graph::CoshNode{arg}, prec);
                        } else if (funcName == "tanh" || funcName == "tanhf") {
                            nodeId = ctx.graph.addNode(graph::TanhNode{arg}, prec);
                        } else {
                            throw std::runtime_error("Unsupported function call: " + funcName);
                        }
                    }
                    // Ternary functions (FMA)
                    else if (callInst->arg_size() == 3) {
                        if (funcName == "llvm.fma.f32" || funcName == "llvm.fma.f64" ||
                            funcName == "llvm.fmuladd.f32" || funcName == "llvm.fmuladd.f64") {
                            graph::NodeId a = getOrCreateNode(callInst->getArgOperand(0), ctx);
                            graph::NodeId b = getOrCreateNode(callInst->getArgOperand(1), ctx);
                            graph::NodeId c = getOrCreateNode(callInst->getArgOperand(2), ctx);
                            nodeId = ctx.graph.addNode(graph::FmaNode{a, b, c}, prec);
                        } else {
                            throw std::runtime_error("Unsupported ternary function: " + funcName);
                        }
                    } else {
                        throw std::runtime_error("Unsupported function arity: " + funcName);
                    }
                    break;
                }

                // Ignore non-floating-point instructions
                default:
                    continue;
            }

            // Map the instruction to the created node
            ctx.valueMap[&inst] = nodeId;

            // If instruction has a name, we could track it (optional)
            if (!inst.getName().empty()) {
                // Could add to a symbol table here if needed
            }
        }
    }

    graph::ComputationGraph LLVMFrontend::parse(const std::filesystem::path& input_path,
                                                  const FrontendOpts& opts) const {
        graph::ComputationGraph graph;

        try {
            // Create LLVM context
            llvm::LLVMContext context;

            // Load the LLVM module
            auto module = loadModule(context, input_path);

            // Find the target function
            llvm::Function* targetFunc = nullptr;

            if (!opts.targetFunction.empty()) {
                targetFunc = findFunction(*module, opts.targetFunction);
                if (!targetFunc) {
                    throw std::runtime_error("Function '" + opts.targetFunction + "' not found in module");
                }
            } else {
                // Use the first non-declaration function
                for (auto& func : *module) {
                    if (!func.isDeclaration()) {
                        targetFunc = &func;
                        break;
                    }
                }

                if (!targetFunc) {
                    throw std::runtime_error("No non-declaration functions found in module");
                }
            }

            if (opts.verbose) {
                std::cout << "Parsing LLVM function: " << targetFunc->getName().str() << std::endl;
            }

            // Create conversion context
            ConversionContext ctx(graph);

            // Parse inputs (function arguments)
            parseInputs(*targetFunc, ctx);

            // Parse expressions (function body)
            parseExpressions(*targetFunc, ctx);

            // Validate the graph
            graph.validate();

            if (opts.verbose) {
                std::cout << "Successfully parsed LLVM IR" << std::endl;
                std::cout << "  Inputs: " << graph.inputs().size() << std::endl;
                std::cout << "  Nodes: " << graph.nodes().size() << std::endl;
                std::cout << "  Outputs: " << graph.outputs().size() << std::endl;
            }

            return graph;

        } catch (const std::exception& e) {
            throw std::runtime_error("LLVM frontend error: " + std::string(e.what()));
        }
    }

}  // namespace frontend::llvm_ir

#endif  // CIRE_LLVM_FRONTEND_ENABLED
