#pragma once

#include "agent/IAgentBackend.hpp"
#include "agent/AgentEvaluator.hpp"
#include "tools/ToolRegistry.hpp"
#include "sprite/Sprite.hpp"
#include "sprite/History.hpp"
#include "animation/Animation.hpp"
#include <functional>
#include <memory>
#include <string>

namespace pixelforge::agent {

struct AgentCallbacks {
    std::function<void(const std::string&)> on_thought;
    std::function<void(const std::string&, const nlohmann::json&)> on_tool_call;
    std::function<void(const tools::ToolResult&)> on_tool_result;
    std::function<void(const EvaluationResult&)> on_evaluation;
    std::function<void(int step, int max_steps)> on_progress;
};

struct OptimizationOptions {
    std::string mode = "smart";          // "smart", "minimal", "full"
    bool compact_tool_output = true;     // Resumo de validação de 1 linha quando válido
    bool compact_history = true;         // Poda turnos anteriores mantendo semântica
    int max_history_turns = 3;           // Número máximo de turnos completos no contexto
};

class AgentLoop {
public:
    AgentLoop(std::shared_ptr<IAgentBackend> backend,
              std::shared_ptr<tools::ToolRegistry> registry);

    void set_callbacks(AgentCallbacks callbacks) { callbacks_ = std::move(callbacks); }
    void set_max_iterations(int max_iter) { max_iterations_ = max_iter; }
    void set_optimization_options(OptimizationOptions opt) { opt_ = std::move(opt); }
    const OptimizationOptions& optimization_options() const { return opt_; }

    std::string process_prompt(const std::string& prompt,
                               sprite::Sprite& sprite,
                               sprite::History& history,
                               std::shared_ptr<animation::Animation>& animation);

    const Conversation& conversation() const { return conversation_; }
    void reset_conversation() { conversation_.clear(); }

private:
    std::shared_ptr<IAgentBackend> backend_;
    std::shared_ptr<tools::ToolRegistry> registry_;
    AgentCallbacks callbacks_;
    int max_iterations_ = 15;
    OptimizationOptions opt_;
    Conversation conversation_;

    std::string build_system_prompt() const;
    std::vector<std::string> select_tools_for_prompt(const std::string& prompt,
                                                    const sprite::Sprite& sprite,
                                                    bool is_animation) const;
};

} // namespace pixelforge::agent
