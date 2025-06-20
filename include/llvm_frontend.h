/**
 * @file include/llvm_frontend.h
 * @brief Definition of the LLVM frontend for converting LLVM IR to a CIRE graph.
 *
 * This file defines functions to parse LLVM IR instructions and convert them into nodes in a CIRE graph,
 * handling various LLVM instructions and types with appropriate conversions and mappings.
 */

#ifndef LLVM_FRONTEND_H
#define LLVM_FRONTEND_H

#include "Graph.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

/// Mapping from LLVM nodes to Cire nodes.
std::map<llvm::Value*, Node*> llvmToCireNodeMap;

/// Mapping from CIRE nodes to LLVM nodes.
std::map<Node*, llvm::Value*> cireToLLVMNodeMap;

/**
 * @brief Adds metadata and mappings for a newly created CIRE node from an LLVM instruction.
 * 
 * @param instr The LLVM instruction being converted.
 * @param graph The CIRE graph to which the node will be added.
 * @param node The CIRE node created from the instruction.
 * 
 * This function sets the rounding type based on the LLVM instruction's type, adds the node to the graph,
 * and maintains mappings between LLVM values and CIRE nodes.
 */
void addDataForCreatedNode(llvm::Instruction& instr, Graph& graph, Node* node);

/**
 * @brief Retrieves or creates a CIRE node corresponding to an LLVM value.
 * 
 * @param val The LLVM value to convert.
 * @param graph The CIRE graph where nodes are stored.
 * @return Node* The corresponding CIRE node, or `nullptr` if conversion fails.
 * 
 * Handles constant values (floating-point and integer) by creating new nodes,
 * and returns existing nodes for non-constant values via the mapping.
 */
Node* getNodeFromLLVMValue(llvm::Value* val, Graph& graph);

/**
 * @brief Parses function arguments from LLVM IR and creates corresponding input nodes in the CIRE graph.
 * 
 * @param graph The CIRE graph to populate.
 * @param func The LLVM function being parsed.
 * 
 * Creates variable nodes for each function argument and adds them to the graph's symbol table and inputs.
 */
void parseInputsInLLVM(Graph& graph, llvm::Function& func);

/**
 * @brief Parses expressions (instructions) from LLVM IR and converts them to CIRE nodes.
 * 
 * @param graph The CIRE graph to populate.
 * @param func The LLVM function being parsed.
 * 
 * Processes each instruction in the function's entry block, converting supported operations
 * (arithmetic, trigonometric, etc.) to corresponding CIRE nodes. Currently limited to single-block functions.
 */
void parseExprsInLLVM(Graph& graph, llvm::Function& func);

#endif
