#include "driver/driver.hpp"
#include "frontend/llvm/llvm_frontend.hpp"

#include <CLI/CLI.hpp>

int main(int argc, char** argv) {
    CLI::App app {"CIRE: Rigorous FP Error Analyser (LLVM IR frontend)"};

    driver::DriverOpts opts;

    app.add_option("input", opts.inputFile, "LLVM IR file (.ll or .bc)")->required()->check(CLI::ExistingFile);
    app.add_option("-d,--domain", opts.domainArgs,
                   "Domain specification (repeatable):\n"
                   "  <file>      - JSON domain file\n"
                   "  <var>=[l,u] - Domain for specific variable");
    app.add_option("--default-domain", opts.defaultDomain, "Default domain for all variables [l,u]");
    app.add_option("--function", opts.targetFunction, "Name of function to analyze")->required();
    app.add_flag("--emit-graph", opts.emitGraph, "Dump computation graph as DOT");
    app.add_flag("--emit-expr", opts.emitExpr, "Dump error expression AST");
    app.add_flag("-v,--verbose", opts.verbose, "Verbose output");

    CLI11_PARSE(app, argc, argv);

    frontend::llvm_ir::LLVMFrontend fe;
    return driver::run(opts, fe) ? 0 : 1;
}
