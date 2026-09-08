#pragma once

#include "sprite/Sprite.hpp"
#include "sprite/History.hpp"
#include "animation/Animation.hpp"
#include "tools/ToolRegistry.hpp"
#include "agent/AgentLoop.hpp"
#include <memory>
#include <string>
#include <vector>

namespace pixelforge::cli {

class CliApp {
public:
    CliApp();

    int run(int argc, char* argv[]);
    void run_interactive_repl();

    void execute_prompt(const std::string& prompt);
    void print_banner() const;
    void print_sprite_preview() const;
    bool export_output(const std::string& filepath);

private:
    sprite::Sprite sprite_;
    sprite::History history_;
    std::shared_ptr<animation::Animation> animation_;
    std::shared_ptr<tools::ToolRegistry> registry_;
    std::shared_ptr<agent::IAgentBackend> backend_;
    std::unique_ptr<agent::AgentLoop> agent_loop_;

    void setup_callbacks();
    void print_help() const;
};

} // namespace pixelforge::cli
