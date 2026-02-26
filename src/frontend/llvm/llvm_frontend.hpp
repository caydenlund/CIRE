#pragma once

#ifdef CIRE_LLVM_FRONTEND_ENABLED

#include "frontend/frontend.hpp"

namespace frontend::llvm_ir {
    class LLVMFrontend final : public Frontend {
    public:
        [[nodiscard]] graph::ComputationGraph parse(const std::filesystem::path& input_path,
                                                    const FrontendOpts& opts) const override;
    };
}  // namespace frontend::llvm_ir

#endif  // CIRE_LLVM_FRONTEND_ENABLED
