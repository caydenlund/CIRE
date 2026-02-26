#include "driver/driver.hpp"
#include "frontend/satire/satire_frontend.hpp"

#include <CLI/CLI.hpp>

int main(int argc, char** argv) {
    CLI::App app {"CIRE: Rigorous FP Error Analyser (SATIRE frontend)"};

    driver::DriverOpts opts;

    app.add_option("input", opts.inputFile, "SATIRE source file")->required()->check(CLI::ExistingFile);
    app.add_option("--domain", opts.domainFile, "JSON file specifying input variable intervals")
            ->required()
            ->check(CLI::ExistingFile);
    app.add_flag("--emit-graph", opts.emitGraph, "Dump computation graph as DOT");
    app.add_flag("--emit-expr", opts.emitExpr, "Dump error expression AST");
    app.add_flag("-v,--verbose", opts.verbose, "Verbose output");

    CLI11_PARSE(app, argc, argv);

    frontend::satire::SatireFrontend fe;
    return driver::run(opts, fe) ? 0 : 1;
}
