#pragma once

#include "tools/ToolResult.hpp"
#include "sprite/Sprite.hpp"
#include "sprite/History.hpp"
#include "third_party/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <memory>

namespace pixelforge::animation {
class Animation;
}

namespace pixelforge::tools {

struct ExecutionContext {
    sprite::Sprite& sprite;
    sprite::History& history;
    std::shared_ptr<animation::Animation> animation;
};

class ITool {
public:
    virtual ~ITool() = default;

    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual nlohmann::json parameter_schema() const = 0;
    virtual std::vector<std::string> required_permissions() const {
        return {"sprite:edit"};
    }
    virtual bool is_modifying() const { return true; }

    virtual ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) = 0;
};

} // namespace pixelforge::tools
