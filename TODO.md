* In generateExpr, remove the common statements like generating the Symbolic expression out of the switch statements
* Investigate memory leaks and think about reducing memory usage.
* Think about metrics to collect and add them to results file
  * Errors on abstracted nodes
* Add conditionals
* Get unhandled fpbench benchmarks to work
* Transfer the Ibex::Interval field from FreeVariable to Variable node and remove the FreeVariable node
* Add an LLVM Instruction field to node.
* Reformat entire codebase using clang-format

# Completed
* Handle fptrunc and fpext LLVM instructions
* Think about metrics to collect and add them to results file
  * Input related metrics
    * Num Operators
    * Height of tree
  * Execution related metrics
    * Time
    * Final Error
  * Scalability studies
    * Abstraction window
    * Num times abstracted
    * Operator Threshold
* Add support for abstraction and other flags to the LLVM frontend
* Add logging support
* Add rounding type to all the operators in Node.cpp
* Output the results of the analysis to a file in CIRE