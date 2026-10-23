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

std::vector<std::string> AgentLoop::select_tools_for_prompt(const std::string& prompt,
                                                          const sprite::Sprite& sprite,
                                                          bool is_animation) const {
    if (opt_.mode == "full") {
        return {}; // Lista vazia instrui ToolRegistry a retornar todas as 43 ferramentas
    }

    if (opt_.mode == "minimal") {
        if (is_animation) {
            return {"create_animation", "add_frame", "duplicate_frame", "move_region", "set_frame_duration", "get_sprite_info"};
        }
        return {
            "create_sprite", "draw_rectangle", "draw_line", "fill_region",
            "set_pixel", "clear_pixel", "create_outline", "add_shadow", "get_sprite_info"
        };
    }

    // Modo "smart" (padrão): filtra dinamicamente um subconjunto de 10 a 15 ferramentas de alta relevância
    if (is_animation) {
        return {
            "create_animation", "add_frame", "duplicate_frame", "insert_frame",
            "delete_frame", "set_frame_duration", "get_frame", "update_frame",
            "compare_frames", "copy_region_between_frames", "interpolate_region",
            "move_region", "get_sprite_info", "draw_rectangle", "set_pixel"
        };
    }

    std::string lower = prompt;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

    // Novo sprite ou canvas ainda em branco
    if (sprite.width() == 0 || sprite.height() == 0 || sprite.calculate_bounding_box().is_empty()) {
        return {
            "create_sprite", "set_palette", "draw_rectangle", "draw_line", "draw_polygon",
            "fill_region", "set_pixel", "clear_pixel", "get_sprite_info",
            "create_outline", "add_shadow", "add_highlight", "create_region", "move_region"
        };
    }

    // Sprite já existente: conjunto base de edição
    std::vector<std::string> tools = {
        "draw_rectangle", "draw_line", "fill_region", "set_pixel",
        "clear_pixel", "get_sprite_info", "create_region", "move_region"
    };

    bool wants_palette = (lower.find("cor") != std::string::npos ||
                          lower.find("color") != std::string::npos ||
                          lower.find("palet") != std::string::npos ||
                          lower.find("quantiz") != std::string::npos ||
                          lower.find("substitu") != std::string::npos ||
                          lower.find("troc") != std::string::npos);

    bool wants_styling = (lower.find("contorn") != std::string::npos ||
                          lower.find("outline") != std::string::npos ||
                          lower.find("sombr") != std::string::npos ||
                          lower.find("shadow") != std::string::npos ||
                          lower.find("brilh") != std::string::npos ||
                          lower.find("light") != std::string::npos ||
                          lower.find("simetr") != std::string::npos ||
                          lower.find("ruid") != std::string::npos ||
                          lower.find("limp") != std::string::npos);

    if (wants_palette) {
        tools.push_back("set_palette");
        tools.push_back("add_color");
        tools.push_back("replace_color");
        tools.push_back("quantize_palette");
    }

    if (wants_styling || !wants_palette) {
        tools.push_back("create_outline");
        tools.push_back("add_shadow");
        tools.push_back("add_highlight");
        tools.push_back("symmetrize_region");
        tools.push_back("clean_pixel_noise");
    }

    if (lower.find("poligon") != std::string::npos || lower.find("forma") != std::string::npos) {
        tools.push_back("draw_polygon");
    }

    if (lower.find("espelh") != std::string::npos || lower.find("flip") != std::string::npos) {
        tools.push_back("flip_horizontal");
        tools.push_back("flip_vertical");
    }

    return tools;
}

std::string AgentLoop::process_prompt(const std::string& prompt,
                                      sprite::Sprite& sprite,
                                      sprite::History& history,
                                      std::shared_ptr<animation::Animation>& animation) {
    if (conversation_.messages().empty()) {
        conversation_.add_message(Message::system(build_system_prompt()));
    } else if (opt_.compact_history) {
        // Poda histórico antigo de turnos para manter contexto enxuto e evitar saturação da janela
        conversation_.compact_history(static_cast<size_t>(opt_.max_history_turns));
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

    // Otimização de contexto: seleciona conjunto inteligente de ferramentas
    std::string lower_prompt = prompt;
    std::transform(lower_prompt.begin(), lower_prompt.end(), lower_prompt.begin(), [](unsigned char c) { return std::tolower(c); });
    bool is_animation = (animation != nullptr || lower_prompt.find("anim") != std::string::npos || lower_prompt.find("frame") != std::string::npos);

    auto allowed_tools = select_tools_for_prompt(prompt, sprite, is_animation);
    auto tools_schema = registry_->get_tools_schema(allowed_tools);
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

            std::string tool_output = res.to_string(opt_.compact_tool_output);
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
