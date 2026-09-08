#pragma once

#include "agent/Conversation.hpp"
#include "third_party/nlohmann/json.hpp"
#include <string>
#include <vector>

namespace pixelforge::agent {

struct AgentResponse {
    std::string content;
    std::vector<ToolCall> tool_calls;
    bool is_finished = true;
};

class IAgentBackend {
public:
    virtual ~IAgentBackend() = default;

    virtual std::string backend_name() const = 0;
    virtual AgentResponse generate(const Conversation& conversation, const nlohmann::json& tools_schema) = 0;
};

} // namespace pixelforge::agent
