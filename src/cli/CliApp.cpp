#include "cli/CliApp.hpp"
#include "tools/PrimitiveTools.hpp"
#include "tools/SemanticTools.hpp"
#include "animation/AnimationTools.hpp"
#include "agent/MockAgentBackend.hpp"
#include "agent/HttpAgentBackend.hpp"
#include "sprite/SpriteState.hpp"
#include "io/PngIO.hpp"
#include "io/GifIO.hpp"
#include "io/SpriteSheetIO.hpp"
#include <iostream>
#include <filesystem>
#include <sstream>

namespace pixelforge::cli {

namespace colors {
    const char* RESET   = "\033[0m";
    const char* BOLD    = "\033[1m";
    const char* CYAN    = "\033[36m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* RED     = "\033[31m";
    const char* MAGENTA = "\033[35m";
    const char* BLUE    = "\033[34m";
    const char* GRAY    = "\033[90m";
}

CliApp::CliApp()
    : sprite_(32, 32, "default_sprite"),
      history_(50) {
    registry_ = std::make_shared<tools::ToolRegistry>();
    tools::register_primitive_tools(*registry_);
    tools::register_semantic_tools(*registry_);
    animation::register_animation_tools(*registry_);

    // Check backend preference
    const char* backend_env = std::getenv("PIXELFORGE_BACKEND");
    if (backend_env && std::string(backend_env) == "http") {
        backend_ = std::make_shared<agent::HttpAgentBackend>();
    } else {
        backend_ = std::make_shared<agent::MockAgentBackend>();
    }

    agent_loop_ = std::make_unique<agent::AgentLoop>(backend_, registry_);
    setup_callbacks();
}

void CliApp::setup_callbacks() {
    agent::AgentCallbacks cb;
    cb.on_thought = [](const std::string& thought) {
        std::cout << colors::CYAN << "[IA Planejamento] " << colors::RESET << thought << "\n";
    };

    cb.on_tool_call = [](const std::string& name, const nlohmann::json& args) {
        std::cout << colors::YELLOW << " ⚙ [TOOL CALL] " << colors::BOLD << name
                  << colors::RESET << colors::GRAY << "(" << args.dump() << ")" << colors::RESET << "\n";
    };

    cb.on_tool_result = [](const tools::ToolResult& res) {
        if (res.success) {
            std::cout << colors::GREEN << "   ✔ " << res.message << colors::RESET << "\n";
        } else {
            std::cout << colors::RED << "   ✖ Falha: " << res.message << colors::RESET << "\n";
        }
    };

    cb.on_evaluation = [](const agent::EvaluationResult& eval) {
        if (eval.passed) {
            std::cout << colors::GREEN << " [Autoavaliação] ✔ Sprite validado sem erros estruturais." << colors::RESET << "\n";
        } else {
            std::cout << colors::MAGENTA << " [Autoavaliação] ⚠ " << eval.issues.size()
                      << " alerta(s) encontrados. Solicitando autocorreção ao agente..." << colors::RESET << "\n";
        }
    };

    agent_loop_->set_callbacks(std::move(cb));
}

void CliApp::print_banner() const {
    std::cout << colors::BOLD << colors::CYAN
              << "╔════════════════════════════════════════════════════════════════╗\n"
              << "║       PixelForge AI - AI-Controlled Pixel Art Engine           ║\n"
              << "╚════════════════════════════════════════════════════════════════╝\n"
              << colors::RESET
              << colors::GRAY << " Backend ativo: " << backend_->backend_name() << "\n"
              << " Ferramentas registradas: " << registry_->tools().size() << "\n"
              << colors::RESET << "\n";
}

void CliApp::print_sprite_preview() const {
    auto summary = sprite::SpriteState::summarize(sprite_, true);
    std::cout << "\n" << colors::BOLD << "--- Estado Atual do Sprite ---" << colors::RESET << "\n";
    std::cout << summary.to_text_format(true) << "\n";

    if (animation_) {
        std::cout << colors::BLUE << "\nAnimação ativa: '" << animation_->name()
                  << "' com " << animation_->frame_count() << " frames ("
                  << animation_->total_duration_ms() << "ms total)\n" << colors::RESET;
    }
}

void CliApp::execute_prompt(const std::string& prompt) {
    std::cout << colors::BOLD << "\nPrompt recebido: \"" << prompt << "\"" << colors::RESET << "\n\n";
    std::string result = agent_loop_->process_prompt(prompt, sprite_, history_, animation_);
    std::cout << "\n" << colors::BOLD << colors::GREEN << "Resultado Final:" << colors::RESET << "\n"
              << result << "\n";
    print_sprite_preview();
}

bool CliApp::export_output(const std::string& filepath) {
    std::filesystem::path p(filepath);
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

    if (ext == ".gif") {
        if (!animation_ || animation_->empty()) {
            std::cout << colors::YELLOW << "Nenhuma animação ativa para exportar como GIF. Criando animação com sprite atual...\n" << colors::RESET;
            auto anim = std::make_shared<animation::Animation>(sprite_.width(), sprite_.height(), "export_anim");
            anim->add_frame(sprite_, 100);
            return io::GifIO::save(*anim, filepath);
        }
        bool ok = io::GifIO::save(*animation_, filepath);
        if (ok) {
            std::cout << colors::GREEN << "✔ Animação GIF salva com sucesso em: " << filepath << colors::RESET << "\n";
        } else {
            std::cout << colors::RED << "✖ Erro ao salvar GIF em: " << filepath << colors::RESET << "\n";
        }
        return ok;
    } else {
        // PNG export
        if (animation_ && animation_->frame_count() > 1 && filepath.find("sheet") != std::string::npos) {
            bool ok = io::SpriteSheetIO::save(*animation_, filepath);
            if (ok) {
                std::cout << colors::GREEN << "✔ Sprite Sheet salva em: " << filepath << colors::RESET << "\n";
            }
            return ok;
        }

        bool ok = io::PngIO::save(sprite_, filepath);
        if (ok) {
            std::cout << colors::GREEN << "✔ Sprite PNG salvo em: " << filepath << colors::RESET << "\n";
        } else {
            std::cout << colors::RED << "✖ Erro ao salvar PNG em: " << filepath << colors::RESET << "\n";
        }
        return ok;
    }
}

void CliApp::print_help() const {
    std::cout << "Comandos suportados:\n"
              << "  create [w] [h] [name]        - Cria um novo canvas de sprite\n"
              << "  open <arquivo.png>           - Abre um arquivo PNG existente\n"
              << "  prompt \"<texto>\"             - Envia uma solicitação para a IA\n"
              << "  animate \"<texto>\"            - Solicita uma animação para a IA\n"
              << "  export <arquivo.png|.gif>    - Exporta sprite, animação GIF ou sprite sheet\n"
              << "  info                         - Exibe o estado textual e preview ASCII\n"
              << "  undo                         - Desfaz a última ação\n"
              << "  redo                         - Refaz a ação desfeita\n"
              << "  repl                         - Inicia sessão interativa\n"
              << "  help                         - Exibe esta ajuda\n"
              << "\nVariáveis de ambiente suportadas:\n"
              << "  PIXELFORGE_BACKEND=http      - Ativa backend HTTP para LLMs locais (Ollama / vLLM)\n"
              << "  PIXELFORGE_API_URL           - Endpoint da API (padrão: http://localhost:11434/v1)\n"
              << "  PIXELFORGE_MODEL             - Nome do modelo (padrão: gemma-4-e4b)\n";
}

void CliApp::run_interactive_repl() {
    print_banner();
    std::cout << colors::GRAY << "Digite seu prompt em linguagem natural ou um comando (ex: 'help', 'exit').\n" << colors::RESET;

    std::string line;
    while (true) {
        std::cout << colors::BOLD << "\npixel-forge> " << colors::RESET;
        if (!std::getline(std::cin, line)) break;

        // Trim leading/trailing whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        std::string cmd = line.substr(start, end - start + 1);

        if (cmd == "exit" || cmd == "quit") {
            std::cout << "Encerrando PixelForge AI. Até logo!\n";
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "info") {
            print_sprite_preview();
        } else if (cmd == "undo") {
            if (history_.can_undo()) {
                sprite_ = history_.undo(sprite_);
                std::cout << colors::GREEN << "Operação desfeita.\n" << colors::RESET;
            } else {
                std::cout << colors::YELLOW << "Nada a desfazer.\n" << colors::RESET;
            }
        } else if (cmd == "redo") {
            if (history_.can_redo()) {
                sprite_ = history_.redo(sprite_);
                std::cout << colors::GREEN << "Operação refeita.\n" << colors::RESET;
            } else {
                std::cout << colors::YELLOW << "Nada a refazer.\n" << colors::RESET;
            }
        } else if (cmd.rfind("export ", 0) == 0) {
            std::string out_path = cmd.substr(7);
            export_output(out_path);
        } else if (cmd.rfind("open ", 0) == 0) {
            std::string in_path = cmd.substr(5);
            try {
                sprite_ = io::PngIO::load(in_path);
                std::cout << colors::GREEN << "Carregado sprite " << sprite_.width() << "x" << sprite_.height() << " de " << in_path << colors::RESET << "\n";
                print_sprite_preview();
            } catch (const std::exception& e) {
                std::cout << colors::RED << "Erro ao abrir PNG: " << e.what() << colors::RESET << "\n";
            }
        } else {
            // Treat as natural language prompt
            execute_prompt(cmd);
        }
    }
}

int CliApp::run(int argc, char* argv[]) {
    if (argc <= 1) {
        run_interactive_repl();
        return 0;
    }

    std::string subcmd = argv[1];

    if (subcmd == "help" || subcmd == "--help" || subcmd == "-h") {
        print_banner();
        print_help();
        return 0;
    }

    if (subcmd == "repl") {
        run_interactive_repl();
        return 0;
    }

    if (subcmd == "create") {
        int w = (argc > 2) ? std::stoi(argv[2]) : 32;
        int h = (argc > 3) ? std::stoi(argv[3]) : 32;
        std::string sname = (argc > 4) ? argv[4] : "sprite";
        sprite_ = sprite::Sprite(w, h, sname);
        std::cout << colors::GREEN << "Sprite criado: " << sname << " (" << w << "x" << h << ")\n" << colors::RESET;
        print_sprite_preview();
        return 0;
    }

    if (subcmd == "open" && argc > 2) {
        try {
            sprite_ = io::PngIO::load(argv[2]);
            std::cout << colors::GREEN << "Carregado sprite de " << argv[2] << colors::RESET << "\n";
            print_sprite_preview();
            return 0;
        } catch (const std::exception& e) {
            std::cerr << colors::RED << "Erro: " << e.what() << colors::RESET << "\n";
            return 1;
        }
    }

    if (subcmd == "prompt" && argc > 2) {
        print_banner();
        execute_prompt(argv[2]);
        // Auto export if requested or export default
        export_output("output.png");
        return 0;
    }

    if (subcmd == "animate" && argc > 2) {
        print_banner();
        execute_prompt(argv[2]);
        export_output("output.gif");
        export_output("output_sheet.png");
        return 0;
    }

    if (subcmd == "export" && argc > 2) {
        return export_output(argv[2]) ? 0 : 1;
    }

    if (subcmd == "info") {
        print_sprite_preview();
        return 0;
    }

    // Default fallback: combine arguments as prompt
    std::string full_prompt;
    for (int i = 1; i < argc; ++i) {
        if (!full_prompt.empty()) full_prompt += " ";
        full_prompt += argv[i];
    }
    print_banner();
    execute_prompt(full_prompt);
    export_output("output.png");
    return 0;
}

} // namespace pixelforge::cli
