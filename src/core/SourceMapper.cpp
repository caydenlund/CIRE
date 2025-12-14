/**
 * @file src/core/SourceMapper.cpp
 * @brief Implementation of SourceMapper class
 */

#include "cire/core/SourceMapper.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>

bool SourceMapper::processModule(llvm::Module* module) {
    if (module == nullptr) return false;

    // Check if module has debug info
    if (module->getNamedMetadata("llvm.dbg.cu") != nullptr) _hasDebugInformation = true;

    // Process all functions in the module
    for (auto& func : *module) {
        if (!func.isDeclaration()) processFunction(&func);
    }

    return _hasDebugInformation;
}

void SourceMapper::processFunction(llvm::Function* func, const std::string& funcName) {
    if (func == nullptr) return;

    std::string actualFuncName = funcName.empty() ? func->getName().str() : funcName;
    _processedFunctions.push_back(actualFuncName);

    // Iterate through basic blocks and instructions
    unsigned int bbIndex = 0;
    for (auto& bb : *func) {
        unsigned int instrIndex = 0;
        for (auto& instr : bb) {
            _totalInstructions++;

            // Check if instruction has debug location
            if (instr.getDebugLoc()) _instructionsWithDebugInfo++;

            instrIndex++;
        }
        bbIndex++;
    }
}

std::unique_ptr<InstructionMetadata> SourceMapper::createMetadata(llvm::Instruction* instr, const std::string& funcName,
                                                                  unsigned int instrIdx, unsigned int bbIdx) {
    auto metadata = std::make_unique<InstructionMetadata>(instr, funcName, instrIdx, bbIdx);
    return metadata;
}

std::unique_ptr<InstructionMetadata> SourceMapper::createMetadata(llvm::Value* value, const std::string& funcName) {
    auto metadata = std::make_unique<InstructionMetadata>(value);
    metadata->functionName = funcName;

    return metadata;
}

std::unique_ptr<InstructionMetadata> SourceMapper::createSyntheticMetadata(const std::string& name,
                                                                           const std::string& type) {
    auto metadata = std::make_unique<InstructionMetadata>();
    metadata->isSynthetic = true;
    metadata->instructionName = name;
    metadata->instructionOpcode = type;
    return metadata;
}

std::map<std::string, unsigned int> SourceMapper::getStatistics() const {
    std::map<std::string, unsigned int> stats;
    stats["total_instructions"] = _totalInstructions;
    stats["instructions_with_debug_info"] = _instructionsWithDebugInfo;
    stats["functions_processed"] = static_cast<unsigned int>(_processedFunctions.size());
    stats["has_debug_info"] = _hasDebugInformation ? 1 : 0;
    return stats;
}
