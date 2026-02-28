#include "driver.hpp"
#include "autodiff/autodiff.hpp"
#include "frontend/frontend.hpp"
#include "interval/interval.hpp"
#include "optimizer/ibex_optimizer.hpp"
#include "report/reporter.hpp"

#include <iostream>
#include <memory>

namespace driver {
    bool run(const DriverOpts& opts, const frontend::Frontend& fe) {
        // 1. Parse → ComputationGraph
        frontend::FrontendOpts fopts {
                .verbose = opts.verbose,
                .emitGraph = opts.emitGraph,
                .targetFunction = opts.targetFunction,
        };
        graph::ComputationGraph g = fe.parse(opts.inputFile, fopts);
        g.validate();

        if (opts.emitGraph) g.dumpDot(std::cout);

        // 2. Load input domain
        interval::InputDomain domain = interval::parseDomainFile(opts.domainFile);

        // 3. Symbolic autodiff → error expression
        autodiff::AutodiffResult ad = autodiff::analyze(g);

        if (opts.emitExpr) ad.expr.dumpAST(std::cout);

        // 4. Select optimizer
        std::unique_ptr<optimizer::Optimizer> opt = std::make_unique<optimizer::IbexOptimizer>();

        optimizer::OptimizerOpts oopts {.verbose = opts.verbose};
        optimizer::OptimizeResult result = opt->maximize(ad.expr, domain, g, ad.symbolicVal, oopts);

        // 5. Report
        report::Reporter reporter(std::cout);
        reporter.print(g, ad.expr, result);

        return true;
    }
}  // namespace driver
