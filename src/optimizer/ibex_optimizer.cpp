#include "ibex_optimizer.hpp"
#include "graph/node.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace optimizer {
    namespace {
        std::string formatOptionValue(double value) {
            std::ostringstream oss;
            oss << std::scientific << std::setprecision(10) << value;
            return oss.str();
        }

        std::string formatOptionValue(bool value) {
            return value ? "true" : "false";
        }

        std::string formatOptionValue(int value) {
            return std::to_string(value);
        }

        void addOption(std::vector<OptimizerOption>& options, std::string name, std::string value) {
            options.push_back(OptimizerOption{std::move(name), std::move(value)});
        }
    }

    void IbexOptimizer::setupSymbols(ConversionContext& ctx) const {
        // Collect all free variables and error variables from the expression
        std::set<std::string> freeVars = ctx.expr.freeVars();
        std::set<std::string> errorVars = ctx.expr.errorVars();

        // Create IBEX symbols for all variables using factory method
        std::vector<const ibex::ExprSymbol*> symbolVec;

        // Add input variables (from domain)
        for (const auto& [name, interval] : ctx.domain) {
            const ibex::ExprSymbol& sym = ibex::ExprSymbol::new_(name.c_str());
            ctx.symbolTable[name] = &sym;
            symbolVec.push_back(&sym);
        }

        // Add error variables (assumed to be in [-1, 1] for simplification)
        for (const auto& name : errorVars) {
            const ibex::ExprSymbol& sym = ibex::ExprSymbol::new_(name.c_str());
            ctx.symbolTable[name] = &sym;
            symbolVec.push_back(&sym);
        }

        // Create IBEX array of symbols
        ctx.symbols.resize(symbolVec.size());
        for (size_t i = 0; i < symbolVec.size(); ++i) {
            ctx.symbols.set_ref(i, *symbolVec[i]);
        }
    }

    const ibex::ExprNode& IbexOptimizer::convertToIbex(
            error_expr::ExprId id,
            ConversionContext& ctx) const {
        using namespace error_expr;

        const ExprKind& kind = ctx.expr.get(id);

        return std::visit(
            [&](auto&& node) -> const ibex::ExprNode& {
                using T = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<T, EVarExpr>) {
                    // First try the symbol table (for input variables)
                    auto it = ctx.symbolTable.find(node.name);
                    if (it != ctx.symbolTable.end()) {
                        return *it->second;
                    }

                    // If not found, check if this is a symbolic variable representing a node result
                    // Find which node this expression ID corresponds to by checking if id matches
                    for (const auto& [nodeId, exprId] : ctx.symbolicVal) {
                        if (exprId == id) {
                            // Found the node - convert it to IBEX
                            return convertNodeToIbex(nodeId, ctx);
                        }
                    }

                    throw std::runtime_error("Unknown variable: " + node.name);
                }
                else if constexpr (std::is_same_v<T, EErrVar>) {
                    auto it = ctx.symbolTable.find(node.name);
                    if (it == ctx.symbolTable.end()) {
                        throw std::runtime_error("Unknown error variable: " + node.name);
                    }
                    return *it->second;
                }
                else if constexpr (std::is_same_v<T, EConst>) {
                    return ibex::ExprConstant::new_scalar(node.value);
                }
                else if constexpr (std::is_same_v<T, EEpsilon>) {
                    // Get unit roundoff for the precision
                    double u = graph::unitRoundoff(node.prec);
                    return ibex::ExprConstant::new_scalar(u);
                }
                else if constexpr (std::is_same_v<T, EAdd>) {
                    const ibex::ExprNode& lhs = convertToIbex(node.lhs, ctx);
                    const ibex::ExprNode& rhs = convertToIbex(node.rhs, ctx);
                    return lhs + rhs;
                }
                else if constexpr (std::is_same_v<T, ESub>) {
                    const ibex::ExprNode& lhs = convertToIbex(node.lhs, ctx);
                    const ibex::ExprNode& rhs = convertToIbex(node.rhs, ctx);
                    return lhs - rhs;
                }
                else if constexpr (std::is_same_v<T, EMul>) {
                    const ibex::ExprNode& lhs = convertToIbex(node.lhs, ctx);
                    const ibex::ExprNode& rhs = convertToIbex(node.rhs, ctx);
                    return lhs * rhs;
                }
                else if constexpr (std::is_same_v<T, EDiv>) {
                    const ibex::ExprNode& lhs = convertToIbex(node.lhs, ctx);
                    const ibex::ExprNode& rhs = convertToIbex(node.rhs, ctx);
                    return lhs / rhs;
                }
                else if constexpr (std::is_same_v<T, ENeg>) {
                    const ibex::ExprNode& src = convertToIbex(node.src, ctx);
                    return -src;
                }
                else if constexpr (std::is_same_v<T, EAbs>) {
                    const ibex::ExprNode& src = convertToIbex(node.src, ctx);
                    return abs(src);
                }
                else if constexpr (std::is_same_v<T, EPow>) {
                    const ibex::ExprNode& base = convertToIbex(node.base, ctx);
                    // IBEX pow requires integer exponent
                    return pow(base, node.exp);
                }
                else {
                    throw std::runtime_error("Unknown expression type in convertToIbex");
                }
            },
            kind);
    }

    const ibex::ExprNode& IbexOptimizer::convertNodeToIbex(
            graph::NodeId nodeId,
            ConversionContext& ctx) const {
        // Check cache first
        auto it = ctx.nodeCache.find(nodeId);
        if (it != ctx.nodeCache.end()) {
            return *it->second;
        }

        const graph::Node& node = ctx.graph.getNode(nodeId);

        const ibex::ExprNode* result = nullptr;

        std::visit(
            graph::Overloaded{
                [&](const graph::InputVarNode& n) {
                    auto symIt = ctx.symbolTable.find(n.name);
                    if (symIt == ctx.symbolTable.end()) {
                        throw std::runtime_error("Unknown input variable: " + n.name);
                    }
                    result = symIt->second;
                },
                [&](const graph::ConstantNode& n) {
                    result = &ibex::ExprConstant::new_scalar(n.value);
                },
                [&](const graph::AddNode& n) {
                    const ibex::ExprNode& lhs = convertNodeToIbex(n.lhs, ctx);
                    const ibex::ExprNode& rhs = convertNodeToIbex(n.rhs, ctx);
                    result = &(lhs + rhs);
                },
                [&](const graph::SubNode& n) {
                    const ibex::ExprNode& lhs = convertNodeToIbex(n.lhs, ctx);
                    const ibex::ExprNode& rhs = convertNodeToIbex(n.rhs, ctx);
                    result = &(lhs - rhs);
                },
                [&](const graph::MulNode& n) {
                    const ibex::ExprNode& lhs = convertNodeToIbex(n.lhs, ctx);
                    const ibex::ExprNode& rhs = convertNodeToIbex(n.rhs, ctx);
                    result = &(lhs * rhs);
                },
                [&](const graph::DivNode& n) {
                    const ibex::ExprNode& lhs = convertNodeToIbex(n.lhs, ctx);
                    const ibex::ExprNode& rhs = convertNodeToIbex(n.rhs, ctx);
                    result = &(lhs / rhs);
                },
                [&](const graph::NegNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &(-src);
                },
                [&](const graph::SqrtNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &sqrt(src);
                },
                [&](const graph::SinNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &sin(src);
                },
                [&](const graph::CosNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &cos(src);
                },
                [&](const graph::ExpNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &exp(src);
                },
                [&](const graph::LogNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &log(src);
                },
                [&](const graph::AbsNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &abs(src);
                },
                [&](const graph::TanNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &tan(src);
                },
                [&](const graph::AsinNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &asin(src);
                },
                [&](const graph::AcosNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &acos(src);
                },
                [&](const graph::AtanNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &atan(src);
                },
                [&](const graph::SinhNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &sinh(src);
                },
                [&](const graph::CoshNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &cosh(src);
                },
                [&](const graph::TanhNode& n) {
                    const ibex::ExprNode& src = convertNodeToIbex(n.src, ctx);
                    result = &tanh(src);
                },
                [&](const graph::FmaNode& n) {
                    const ibex::ExprNode& a = convertNodeToIbex(n.a, ctx);
                    const ibex::ExprNode& b = convertNodeToIbex(n.b, ctx);
                    const ibex::ExprNode& c = convertNodeToIbex(n.c, ctx);
                    result = &(a * b + c);
                },
                [&](const auto&) {
                    throw std::runtime_error("Unsupported node type in convertNodeToIbex");
                },
            },
            node.kind);

        ctx.nodeCache[nodeId] = result;
        return *result;
    }

    OptimizeResult IbexOptimizer::maximize(
            const error_expr::ErrorExpr& expr,
            const interval::InputDomain& domain,
            const graph::ComputationGraph& graph,
            const std::unordered_map<graph::NodeId, error_expr::ExprId>& symbolicVal,
            const OptimizerOpts& opts) const {
        // Setup conversion context
        ConversionContext ctx {
            .expr = expr,
            .domain = domain,
            .graph = graph,
            .symbolicVal = symbolicVal,
        };
        setupSymbols(ctx);

        // Convert error expression to IBEX
        const ibex::ExprNode& ibexExpr = convertToIbex(expr.root(), ctx);

        // Build optimization system using SystemFactory
        ibex::SystemFactory factory;

        // Add all variables to the system
        for (int i = 0; i < ctx.symbols.size(); ++i) {
            factory.add_var(ctx.symbols[i]);
        }

        // Add goal: maximize f(x) = minimize -f(x)
        const ibex::ExprNode& ibexGoalExpr = -ibexExpr;
        factory.add_goal(ibexGoalExpr);

        std::ostringstream expression;
        expression << ibexGoalExpr;

        // Create system
        ibex::System system(factory);

        // Setup input intervals
        ibex::IntervalVector box(ctx.symbols.size());
        int idx = 0;

        // Set intervals for input variables
        for (const auto& [name, interval] : domain) {
            auto it = ctx.symbolTable.find(name);
            if (it != ctx.symbolTable.end()) {
                // Find the index of this symbol in the symbols array
                for (int i = 0; i < ctx.symbols.size(); ++i) {
                    if (&ctx.symbols[i] == it->second) {
                        box[i] = ibex::Interval(interval.lo, interval.hi);
                        break;
                    }
                }
            }
        }

        // Set intervals for error variables (assumed [-1, 1])
        std::set<std::string> errorVars = expr.errorVars();
        for (const auto& name : errorVars) {
            auto it = ctx.symbolTable.find(name);
            if (it != ctx.symbolTable.end()) {
                for (int i = 0; i < ctx.symbols.size(); ++i) {
                    if (&ctx.symbols[i] == it->second) {
                        box[i] = ibex::Interval(-1.0, 1.0);
                        break;
                    }
                }
            }
        }

        // Configure optimizer
        ibex::DefaultOptimizerConfig optConfig(
            system,
            opts.relTolerance,  // Use user-specified relative tolerance
            opts.relTolerance * 1e-10,  // Absolute tolerance (much smaller)
            ibex::NormalizedSystem::default_eps_h,     // constraint tolerance: 1e-8
            false,  // rigor (rigorous mode disabled for performance)
            ibex::DefaultOptimizerConfig::default_inHC4,
            false,  // kkt
            ibex::DefaultOptimizerConfig::default_random_seed,
            ibex::OptimizerConfig::default_eps_x
        );

        // Set timeout if specified
        if (opts.timeoutSeconds > 0) {
            optConfig.set_timeout(opts.timeoutSeconds);
        }

        // Create and run optimizer
        ibex::Optimizer opt(optConfig);

        std::vector<OptimizerOption> optimizerOptions;
        addOption(optimizerOptions, "rel_eps_f", formatOptionValue(optConfig.get_rel_eps_f()));
        addOption(optimizerOptions, "abs_eps_f", formatOptionValue(optConfig.get_abs_eps_f()));
        addOption(optimizerOptions, "eps_h", formatOptionValue(optConfig.get_eps_h()));
        addOption(optimizerOptions, "rigor", formatOptionValue(optConfig.with_rigor()));
        addOption(optimizerOptions, "inHC4", formatOptionValue(optConfig.with_inHC4()));
        addOption(optimizerOptions, "kkt", formatOptionValue(optConfig.with_kkt()));
        addOption(optimizerOptions, "random_seed", formatOptionValue(optConfig.get_random_seed()));
        addOption(optimizerOptions, "eps_x", formatOptionValue(optConfig.get_eps_x()));
        addOption(optimizerOptions, "timeout", formatOptionValue(optConfig.get_timeout()));
        addOption(optimizerOptions, "trace", formatOptionValue(optConfig.get_trace()));
        addOption(optimizerOptions, "extended_cov", formatOptionValue(optConfig.with_extended_cov()));
        addOption(optimizerOptions, "anticipated_upper_bounding",
                  formatOptionValue(optConfig.with_anticipated_upper_bounding()));

        if (opts.verbose) {
            std::cout << "Running IBEX optimizer..." << std::endl;
            std::cout << "  Variables: " << ctx.symbols.size() << std::endl;
            std::cout << "  Timeout: " << opts.timeoutSeconds << "s" << std::endl;
        }

        opt.optimize(box);

        // Extract results
        OptimizeResult result;
        result.optimizerName = "ibex";
        result.optimizationExpression = expression.str();
        result.optimizerOptions = std::move(optimizerOptions);

        switch (opt.get_status()) {
            case ibex::Optimizer::SUCCESS:
                // Optimizer found a solution
                // Note: IBEX minimizes, so we negate to get maximum
                result.upperBound = -opt.get_uplo();  // uplo is the lower bound on minimum
                result.provedTight = false;  // Conservative: not rigorously proven

                // Extract witness point
                if (opt.get_loup_point().size() > 0) {
                    ibex::IntervalVector witnessPoint = opt.get_loup_point();
                    // Map back to variable names
                    int varIdx = 0;
                    for (const auto& [name, interval] : domain) {
                        auto it = ctx.symbolTable.find(name);
                        if (it != ctx.symbolTable.end()) {
                            for (int i = 0; i < ctx.symbols.size(); ++i) {
                                if (&ctx.symbols[i] == it->second) {
                                    result.witnessInputs[name] = witnessPoint[i].mid();
                                    break;
                                }
                            }
                        }
                    }

                    // Evaluate output value at witness point using simple forward evaluation
                    if (!graph.outputs().empty()) {
                        std::unordered_map<graph::NodeId, double> nodeValues;

                        // Set input values
                        for (const auto& [name, nodeId] : graph.inputs()) {
                            if (result.witnessInputs.find(name) != result.witnessInputs.end()) {
                                nodeValues[nodeId] = result.witnessInputs[name];
                            }
                        }

                        // Evaluate in topological order
                        for (graph::NodeId id : graph.topoOrder()) {
                            const graph::Node& node = graph.getNode(id);

                            std::visit(
                                graph::Overloaded{
                                    [&](const graph::InputVarNode&) {
                                        // Already set above
                                    },
                                    [&](const graph::ConstantNode& n) {
                                        nodeValues[id] = n.value;
                                    },
                                    [&](const graph::AddNode& n) {
                                        nodeValues[id] = nodeValues[n.lhs] + nodeValues[n.rhs];
                                    },
                                    [&](const graph::SubNode& n) {
                                        nodeValues[id] = nodeValues[n.lhs] - nodeValues[n.rhs];
                                    },
                                    [&](const graph::MulNode& n) {
                                        nodeValues[id] = nodeValues[n.lhs] * nodeValues[n.rhs];
                                    },
                                    [&](const graph::DivNode& n) {
                                        nodeValues[id] = nodeValues[n.lhs] / nodeValues[n.rhs];
                                    },
                                    [&](const graph::NegNode& n) {
                                        nodeValues[id] = -nodeValues[n.src];
                                    },
                                    [&](const graph::SqrtNode& n) {
                                        nodeValues[id] = std::sqrt(nodeValues[n.src]);
                                    },
                                    [&](const graph::SinNode& n) {
                                        nodeValues[id] = std::sin(nodeValues[n.src]);
                                    },
                                    [&](const graph::CosNode& n) {
                                        nodeValues[id] = std::cos(nodeValues[n.src]);
                                    },
                                    [&](const graph::ExpNode& n) {
                                        nodeValues[id] = std::exp(nodeValues[n.src]);
                                    },
                                    [&](const graph::LogNode& n) {
                                        nodeValues[id] = std::log(nodeValues[n.src]);
                                    },
                                    [&](const graph::FmaNode& n) {
                                        nodeValues[id] = nodeValues[n.a] * nodeValues[n.b] + nodeValues[n.c];
                                    },
                                    [&](const auto&) {
                                        // Other node types - set to 0 as fallback
                                        nodeValues[id] = 0.0;
                                    },
                                },
                                node.kind);
                        }

                        // Get output value
                        graph::NodeId outputId = graph.outputs().front();
                        if (nodeValues.find(outputId) != nodeValues.end()) {
                            result.witnessOutputValue = nodeValues[outputId];
                        }
                    }
                }

                if (opts.verbose) {
                    std::cout << "Optimization SUCCESS: bound = " << result.upperBound << std::endl;
                    std::cout << "  Time: " << opt.get_time() << "s" << std::endl;
                }
                break;

            case ibex::Optimizer::INFEASIBLE:
                throw std::runtime_error("IBEX: Problem is infeasible (no feasible point exists)");

            case ibex::Optimizer::NO_FEASIBLE_FOUND:
                throw std::runtime_error("IBEX: No feasible point found");

            case ibex::Optimizer::UNBOUNDED_OBJ:
                throw std::runtime_error("IBEX: Objective is unbounded");

            case ibex::Optimizer::TIME_OUT:
                // Return best bound found so far
                result.upperBound = -opt.get_uplo();
                result.provedTight = false;
                if (opts.verbose) {
                    std::cout << "Optimization TIMEOUT: best bound = " << result.upperBound << std::endl;
                }
                break;

            case ibex::Optimizer::UNREACHED_PREC:
                // Could not reach requested precision, but have a bound
                result.upperBound = -opt.get_uplo();
                result.provedTight = false;
                if (opts.verbose) {
                    std::cout << "Optimization UNREACHED_PREC: bound = " << result.upperBound << std::endl;
                }
                break;

            default:
                throw std::runtime_error("IBEX: Unknown optimizer status");
        }

        return result;
    }

}  // namespace optimizer
