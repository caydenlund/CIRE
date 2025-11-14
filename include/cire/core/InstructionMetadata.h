/**
 * @file include/cire/core/InstructionMetadata.h
 * @brief Metadata for tracking LLVM IR instructions and source locations
 *
 * This file defines structures to store comprehensive information about LLVM instructions,
 * including source location, instruction details, and hierarchical context.
 */

#ifndef CIRE_INSTRUCTIONMETADATA_H
#define CIRE_INSTRUCTIONMETADATA_H

#include <string>

namespace llvm {
    class Value;
    class Instruction;
    class DebugLoc;
}

/**
 * @brief Source location information extracted from LLVM debug metadata
 */
struct SourceLocation {
    std::string filename;
    unsigned int line = 0;
    unsigned int column = 0;

    bool isValid() const { return line > 0; }

    std::string toString() const {
        if (!isValid()) return "<unknown location>";
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
};

/**
 * @brief Comprehensive metadata for a LLVM instruction in the CIRE graph
 *
 * Stores all relevant information about the original LLVM instruction that
 * generated a CIRE node, including source attribution and instruction details.
 */
class InstructionMetadata {
public:
    // LLVM instruction pointer (may be null for synthetic nodes)
    llvm::Value* llvmValue = nullptr;

    // Instruction identification
    std::string instructionName;      // LLVM value name (e.g., "%add1")
    std::string instructionOpcode;    // Opcode name (e.g., "fadd", "fmul")
    std::string irRepresentation;     // Full LLVM IR string

    // Source location (from debug info)
    SourceLocation sourceLocation;

    // Context information
    std::string functionName;         // Parent function name
    unsigned int instructionIndex = 0; // Position in basic block
    unsigned int basicBlockIndex = 0;  // Basic block number

    // Node is synthetic (not from LLVM) if true
    bool isSynthetic = false;

    InstructionMetadata() = default;

    explicit InstructionMetadata(llvm::Value* value);

    InstructionMetadata(llvm::Value* value, const std::string& funcName,
                        unsigned int instrIdx, unsigned int bbIdx);

    /**
     * @brief Extract debug location from LLVM instruction
     * @param instr The LLVM instruction to extract location from
     * @return true if debug location was successfully extracted
     */
    bool extractDebugLocation(llvm::Instruction* instr);

    /**
     * @brief Extract full IR representation of the instruction
     * @param value The LLVM value to convert to string
     */
    void extractIRRepresentation(llvm::Value* value);

    /**
     * @brief Check if this metadata has valid LLVM instruction
     * @return true if metadata references a valid LLVM instruction
     */
    bool hasLLVMInstruction() const { return llvmValue != nullptr && !isSynthetic; }

    /**
     * @brief Get a human-readable identifier for this instruction
     * @return String identifier suitable for display
     */
    std::string getDisplayName() const;

    /**
     * @brief Get a unique identifier for this instruction
     * @return String that uniquely identifies this instruction
     */
    std::string getUniqueId() const;
};

#endif  // CIRE_INSTRUCTIONMETADATA_H
