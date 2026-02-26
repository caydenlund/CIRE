#pragma once

#include "frontend/frontend.hpp"

namespace frontend::satire {
    class SatireFrontend final : public Frontend {
    public:
        [[nodiscard]] graph::ComputationGraph parse(const std::filesystem::path& input_path,
                                                    const FrontendOpts& opts) const override;
    };
}  // namespace frontend::satire
