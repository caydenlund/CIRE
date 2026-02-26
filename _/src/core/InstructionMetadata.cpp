/**
 * @file src/core/InstructionMetadata.cpp
 * @brief Implementation of InstructionMetadata class
 */

#include "cire/core/InstructionMetadata.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

InstructionMetadata::InstructionMetadata(llvm::Value* value) : llvmValue(value) {
    if (value) {
        instructionName = value->getName().str();
        if (auto* instr = llvm::dyn_cast<llvm::Instruction>(value)) {
            instructionOpcode = instr->getOpcodeName();
            extractDebugLocation(instr);
        }
        extractIRRepresentation(value);
    }
}

InstructionMetadata::InstructionMetadata(llvm::Value* value, const std::string& funcName,
                                         unsigned int instrIdx, unsigned int bbIdx)
    : llvmValue(value), functionName(funcName), instructionIndex(instrIdx), basicBlockIndex(bbIdx) {
    if (value) {
        instructionName = value->getName().str();
        if (auto* instr = llvm::dyn_cast<llvm::Instruction>(value)) {
            instructionOpcode = instr->getOpcodeName();
            extractDebugLocation(instr);
        }
        extractIRRepresentation(value);
    }
}

bool InstructionMetadata::extractDebugLocation(llvm::Instruction* instr) {
    if (!instr) return false;

    const llvm::DebugLoc& debugLoc = instr->getDebugLoc();
    if (!debugLoc) return false;

    sourceLocation.line = debugLoc.getLine();
    sourceLocation.column = debugLoc.getCol();

    if (auto* scope = llvm::dyn_cast_or_null<llvm::DIScope>(debugLoc.getScope())) {
        sourceLocation.filename = scope->getFilename().str();
    }

    return true;
}

void InstructionMetadata::extractIRRepresentation(llvm::Value* value) {
    if (!value) return;

    std::string str;
    llvm::raw_string_ostream rso(str);
    value->print(rso);
    irRepresentation = rso.str();
}

std::string InstructionMetadata::getDisplayName() const {
    if (isSynthetic) {
        return "<synthetic>";
    }

    if (!instructionName.empty()) {
        return instructionName;
    }

    if (!instructionOpcode.empty()) {
        return "<" + instructionOpcode + ">";
    }

    return "<unnamed>";
}

std::string InstructionMetadata::getUniqueId() const {
    if (isSynthetic) {
        return "synthetic_" + instructionName;
    }

    std::string id = functionName + "_bb" + std::to_string(basicBlockIndex)
                   + "_instr" + std::to_string(instructionIndex);

    if (!instructionName.empty()) {
        id += "_" + instructionName;
    } else if (!instructionOpcode.empty()) {
        id += "_" + instructionOpcode;
    }

    return id;
}
