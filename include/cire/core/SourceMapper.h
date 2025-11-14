/**
 * @file include/cire/core/SourceMapper.h
 * @brief Utility for mapping LLVM IR to source locations and extracting debug info
 *
 * Provides centralized functionality for extracting and managing debug information
 * from LLVM modules, mapping instructions to source locations, and querying
 * instruction metadata.
 */

#ifndef CIRE_SOURCEMAPPER_H
#define CIRE_SOURCEMAPPER_H

#include "cire/core/InstructionMetadata.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace llvm {
    class Module;
    class Function;
    class Instruction;
    class Value;
}

/**
 * @brief Manages mapping between LLVM IR and source code locations
 *
 * Extracts and indexes debug information from LLVM modules to enable
 * efficient queries about instruction locations and context.
 */
class SourceMapper {
public:
    SourceMapper() = default;
    ~SourceMapper() = default;

    /**
     * @brief Process an LLVM module to extract all debug information
     * @param module The LLVM module to process
     * @return true if debug info was found and processed
     */
    bool processModule(llvm::Module* module);

    /**
     * @brief Process a single LLVM function to extract instruction metadata
     * @param func The LLVM function to process
     * @param funcName Optional override for function name
     */
    void processFunction(llvm::Function* func, const std::string& funcName = "");

    /**
     * @brief Create metadata for a specific LLVM instruction
     * @param instr The instruction to create metadata for
     * @param funcName Parent function name
     * @param instrIdx Instruction index in basic block
     * @param bbIdx Basic block index
     * @return Unique pointer to created InstructionMetadata
     */
    std::unique_ptr<InstructionMetadata> createMetadata(llvm::Instruction* instr,
                                                        const std::string& funcName,
                                                        unsigned int instrIdx,
                                                        unsigned int bbIdx);

    /**
     * @brief Create metadata for an LLVM value (may be constant or instruction)
     * @param value The LLVM value
     * @param funcName Parent function name
     * @return Unique pointer to created InstructionMetadata
     */
    std::unique_ptr<InstructionMetadata> createMetadata(llvm::Value* value,
                                                        const std::string& funcName = "");

    /**
     * @brief Create synthetic metadata for nodes not derived from LLVM
     * @param name Synthetic node name
     * @param type Node type description
     * @return Unique pointer to created InstructionMetadata
     */
    std::unique_ptr<InstructionMetadata> createSyntheticMetadata(const std::string& name,
                                                                 const std::string& type);

    /**
     * @brief Check if module has debug information
     * @return true if debug info is available
     */
    bool hasDebugInfo() const { return hasDebugInformation; }

    /**
     * @brief Get statistics about processed instructions
     * @return Map of statistic name to value
     */
    std::map<std::string, unsigned int> getStatistics() const;

private:
    bool hasDebugInformation = false;
    unsigned int totalInstructions = 0;
    unsigned int instructionsWithDebugInfo = 0;
    std::vector<std::string> processedFunctions;
};

#endif  // CIRE_SOURCEMAPPER_H
