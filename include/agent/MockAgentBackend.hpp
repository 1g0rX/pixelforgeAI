#pragma once

#include "agent/IAgentBackend.hpp"
#include <string>
#include <vector>
#include <queue>

namespace pixelforge::agent {

class MockAgentBackend : public IAgentBackend {
public:
    MockAgentBackend();

    std::string backend_name() const override { return "MockAgent (Rule-based / Procedural Planner)"; }

    AgentResponse generate(const Conversation& conversation, const nlohmann::json& tools_schema) override;

    void set_custom_plan(std::vector<std::vector<ToolCall>> steps);
    void reset_state();

private:
    int step_index_ = 0;
    std::vector<std::vector<ToolCall>> custom_steps_;
    bool plan_generated_ = false;

    void generate_plan_for_prompt(const std::string& prompt, const std::string& current_state_text);
    std::string to_lower(std::string_view str) const;
};

} // namespace pixelforge::agent
