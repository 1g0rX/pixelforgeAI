#pragma once

#include "tools/ITool.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <string>

namespace pixelforge::tools {

class ToolRegistry {
public:
    ToolRegistry();

    void register_tool(std::unique_ptr<ITool> tool);
    bool has_tool(const std::string& name) const;
    ITool* get_tool(const std::string& name);
    const ITool* get_tool(const std::string& name) const;

    // Permissions
    void grant_permission(const std::string& perm) { permissions_.insert(perm); }
    void revoke_permission(const std::string& perm) { permissions_.erase(perm); }
    bool has_permission(const std::string& perm) const;

    // Validation & Execution
    ToolResult execute_tool(const std::string& name, ExecutionContext& ctx, const nlohmann::json& args);

    // Schemas for AI Agents (OpenAI/Ollama format)
    nlohmann::json get_tools_schema() const;
    std::string get_tools_documentation() const;

    const std::unordered_map<std::string, std::unique_ptr<ITool>>& tools() const {
        return tools_;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ITool>> tools_;
    std::unordered_set<std::string> permissions_;

    std::string validate_arguments(const ITool& tool, const nlohmann::json& args) const;
};

} // namespace pixelforge::tools
