#include "agent/AgentLoop.hpp"
#include "sprite/SpriteState.hpp"
#include "core/Logger.hpp"
#include <sstream>

namespace pixelforge::agent {

AgentLoop::AgentLoop(std::shared_ptr<IAgentBackend> backend,
                     std::shared_ptr<tools::ToolRegistry> registry)
    : backend_(std::move(backend)), registry_(std::move(registry)) {}

std::string AgentLoop::build_system_prompt() const {
    std::ostringstream ss;
    ss << "Você é o agente inteligente do PixelForge AI, um especialista em pixel art e animação retrô.\n"
       << "Você NÃO gera imagens diretamente via difusão. Em vez disso, você raciocina e controla um Sprite Engine determinístico através de chamadas a ferramentas estruturadas.\n"
       << "\n"
       << "REGRAS CRÍTICAS DE PIXEL ART:\n"
       << "1. Coordenadas inteiras sem anti-aliasing (apenas pixels totalmente sólidos a=255 ou transparentes a=0).\n"
       << "2. Economia de cores: use paletas coesas e consistentes (máximo 8 a 16 cores por personagem).\n"
       << "3. Silhueta e legibilidade: crie formas reconhecíveis e contornos limpos.\n"
       << "4. Regiões semânticas: agrupe partes do personagem (ex: 'head', 'body', 'left_leg', 'weapon') para permitir animação e movimentação fáceis com move_region.\n"
       << "5. Prefira ferramentas de alto nível quando apropriado: create_outline, add_shadow, add_highlight, move_region, quantize_palette.\n"
       << "\n"
       << "COMO VOCÊ PERCEBE O SPRITE:\n"
       << "Você não precisa de visão de computador nem de imagem visual. O Sprite Engine fornece uma representação estruturada completa com resolução, paleta, contadores, bounding box e uma matriz ASCII compacta.\n"
       << "Analise o estado após cada ferramenta e efetue correções se houver ruído ou problemas de alinhamento.";
    return ss.str();
}

std::string AgentLoop::process_prompt(const std::string& prompt,
                                      sprite::Sprite& sprite,
                                      sprite::History& history,
                                      std::shared_ptr<animation::Animation>& animation) {
    if (conversation_.messages().empty()) {
        conversation_.add_message(Message::system(build_system_prompt()));
    }

    // Include current sprite state in prompt message
    auto state_summary = sprite::SpriteState::summarize(sprite, true);
    std::string user_msg = "Prompt do Usuário: \"" + prompt + "\"\n\n" +
                           state_summary.to_text_format(true);
    conversation_.add_message(Message::user(std::move(user_msg)));

    tools::ExecutionContext ctx{
        .sprite = sprite,
        .history = history,
        .animation = animation
    };

    auto tools_schema = registry_->get_tools_schema();
    int iteration = 0;
    std::string final_message;

    while (iteration < max_iterations_) {
        iteration++;
        if (callbacks_.on_progress) {
            callbacks_.on_progress(iteration, max_iterations_);
        }

        AgentResponse resp = backend_->generate(conversation_, tools_schema);

        if (!resp.content.empty() && callbacks_.on_thought) {
            callbacks_.on_thought(resp.content);
        }

        conversation_.add_message(Message::assistant(resp.content, resp.tool_calls));

        if (resp.tool_calls.empty()) {
            // Evaluate current state
            EvaluationResult eval = ctx.animation ?
                AgentEvaluator::evaluate_animation(*ctx.animation) :
                AgentEvaluator::evaluate_sprite(ctx.sprite);

            if (callbacks_.on_evaluation) {
                callbacks_.on_evaluation(eval);
            }

            if (!eval.passed && iteration < max_iterations_ - 1) {
                // Ask agent to correct issues found by evaluator
                conversation_.add_message(Message::user(eval.to_text_critique() + "\nPor favor, execute as correções necessárias."));
                continue;
            }

            final_message = resp.content;
            break;
        }

        // Execute tool calls
        for (const auto& tc : resp.tool_calls) {
            if (callbacks_.on_tool_call) {
                callbacks_.on_tool_call(tc.name, tc.arguments);
            }

            tools::ToolResult res = registry_->execute_tool(tc.name, ctx, tc.arguments);

            if (callbacks_.on_tool_result) {
                callbacks_.on_tool_result(res);
            }

            std::string tool_output = res.to_string();
            conversation_.add_message(Message::tool(tool_output, tc.id));
        }

        // Keep active animation reference in sync
        animation = ctx.animation;
    }

    animation = ctx.animation;
    if (final_message.empty()) {
        final_message = "Execução concluída após " + std::to_string(iteration) + " iterações.";
    }

    return final_message;
}

} // namespace pixelforge::agent
