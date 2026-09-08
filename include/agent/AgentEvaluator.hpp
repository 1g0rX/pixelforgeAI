#pragma once

#include "sprite/Sprite.hpp"
#include "core/Types.hpp"
#include "animation/Animation.hpp"
#include <string>
#include <vector>

namespace pixelforge::agent {

struct EvaluationResult {
    bool passed = true;
    int issue_count = 0;
    std::vector<std::string> issues;
    std::vector<std::string> recommendations;

    std::string to_text_critique() const;
};

class AgentEvaluator {
public:
    static EvaluationResult evaluate_sprite(const sprite::Sprite& sprite, size_t max_expected_colors = 16);
    static EvaluationResult evaluate_animation(const animation::Animation& anim);
};

} // namespace pixelforge::agent
