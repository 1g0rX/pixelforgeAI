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
#include "core/Config.hpp"
#include <iostream>
#include <fstream>
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
    // Carrega configuração centralizada
    config_ = core::Config::discover_and_load();

    // Usa tamanho default do config para o sprite inicial
    sprite_ = sprite::Sprite(config_.default_sprite_width, config_.default_sprite_height, "default_sprite");

    registry_ = std::make_shared<tools::ToolRegistry>();
    tools::register_primitive_tools(*registry_);
    tools::register_semantic_tools(*registry_);
    animation::register_animation_tools(*registry_);

    init_backend();
}

void CliApp::init_backend() {
    if (config_.backend == "http") {
        agent::HttpBackendConfig http_cfg;
        http_cfg.base_url        = config_.api_url;
        http_cfg.api_key         = config_.api_key;
        http_cfg.model           = config_.model;
        http_cfg.temperature     = config_.temperature;
        http_cfg.timeout_seconds = config_.timeout_seconds;
        backend_ = std::make_shared<agent::HttpAgentBackend>(http_cfg);
    } else {
        backend_ = std::make_shared<agent::MockAgentBackend>();
    }

    agent_loop_ = std::make_unique<agent::AgentLoop>(backend_, registry_);
    agent_loop_->set_max_iterations(config_.max_agent_iterations);

    agent::OptimizationOptions opt;
    opt.mode = config_.context_mode;
    opt.max_history_turns = config_.max_history_turns;
    opt.compact_tool_output = config_.compact_tool_output;
    opt.compact_history = (config_.context_mode != "full");
    agent_loop_->set_optimization_options(opt);

    setup_callbacks();
}

bool CliApp::switch_profile(const std::string& profile_name) {
    std::string config_path = "pixelforge.json";
    if (!std::filesystem::exists(config_path)) {
        if (const char* home = std::getenv("HOME")) {
            config_path = std::string(home) + "/.config/pixelforge/pixelforge.json";
        }
    }

    if (std::filesystem::exists(config_path)) {
        std::ifstream file(config_path);
        if (file.is_open()) {
            try {
                nlohmann::json j = nlohmann::json::parse(file, nullptr, true, true, true);
                if (j.contains("profiles") && j["profiles"].contains(profile_name)) {
                    const auto& p = j["profiles"][profile_name];
                    if (p.contains("type"))            config_.backend = p["type"].get<std::string>();
                    if (p.contains("api_url"))         config_.api_url = p["api_url"].get<std::string>();
                    if (p.contains("api_key"))         config_.api_key = p["api_key"].get<std::string>();
                    if (p.contains("model"))           config_.model   = p["model"].get<std::string>();
                    if (p.contains("temperature"))     config_.temperature = p["temperature"].get<double>();
                    if (p.contains("timeout_seconds")) config_.timeout_seconds = p["timeout_seconds"].get<int>();
                    config_.active_profile = profile_name;
                    init_backend();
                    return true;
                }
            } catch (...) {}
        }
    }
    return false;
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
              << " Modelo: " << config_.model << "\n"
              << " Sprite máximo: " << config_.max_sprite_width << "x" << config_.max_sprite_height << "\n"
              << " Ferramentas registradas: " << registry_->tools().size() << "\n"
              << " Otimização de contexto: " << config_.context_mode << " (economia de tokens ativa)\n"
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
              << "  open <arquivo.png>           - Abre um arquivo PNG existente para análise/edição\n"
              << "  prompt \"<texto>\"             - Envia uma solicitação para a IA\n"
              << "  animate \"<texto>\"            - Solicita uma animação para a IA\n"
              << "  export <arquivo.png|.gif>    - Exporta sprite, animação GIF ou sprite sheet\n"
              << "  info                         - Exibe o estado textual, paleta e preview ASCII\n"
              << "  profile <nome>               - Troca de provedor/modelo em tempo de execução (ex: profile lmstudio)\n"
              << "  config                       - Exibe a configuração ativa em JSON\n"
              << "  config init                  - Gera um pixelforge.json de exemplo no diretório atual\n"
              << "  undo                         - Desfaz a última ação\n"
              << "  redo                         - Refaz a ação desfeita\n"
              << "  repl                         - Inicia sessão interativa\n"
              << "  help                         - Exibe esta ajuda\n"
              << "\nOpções de linha de comando:\n"
              << "  --profile, -p <nome>         - Seleciona o perfil de modelo ativo (ex: --profile lmstudio)\n"
              << "\nPerfis pré-configurados em pixelforge.json:\n"
              << "  ollama, lmstudio, vllm, llamacpp, jan, textgen_webui, openrouter, openai, groq, mock\n"
              << "\nConfiguração (pixelforge.json - lido em tempo de execução SEM compilar):\n"
              << "  Basta editar pixelforge.json e rodar o programa. Nenhuma compilação é necessária!\n"
              << "  O PixelForge procura pixelforge.json em:\n"
              << "    1. Diretório atual (./pixelforge.json)\n"
              << "    2. ~/.config/pixelforge/pixelforge.json\n"
              << "  Variáveis de ambiente sobrescrevem o arquivo.\n"
              << "\nVariáveis de ambiente suportadas:\n"
              << "  PIXELFORGE_BACKEND=http      - Ativa backend HTTP para LLMs locais (Ollama / vLLM)\n"
              << "  PIXELFORGE_API_URL           - Endpoint da API (padrão: http://localhost:11434/v1)\n"
              << "  PIXELFORGE_MODEL             - Nome do modelo (padrão: gemma-4-e4b)\n"
              << "  PIXELFORGE_API_KEY           - Chave de API (padrão: none)\n"
              << "  PIXELFORGE_MAX_SPRITE        - Tamanho máximo de sprite (ex: 1024 = 1024x1024)\n";
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
        } else if (cmd == "config init") {
            auto example = core::Config::example_json();
            std::ofstream out("pixelforge.json");
            if (out.is_open()) {
                out << example.dump(2) << "\n";
                out.close();
                std::cout << colors::GREEN << "✔ Arquivo pixelforge.json gerado no diretório atual.\n"
                          << colors::GRAY << "  Edite-o para configurar modelo, backend, limites, etc.\n" << colors::RESET;
            } else {
                std::cout << colors::RED << "✖ Erro ao criar pixelforge.json\n" << colors::RESET;
            }
        } else if (cmd == "config") {
            std::cout << colors::BOLD << "Configuração ativa:" << colors::RESET << "\n"
                      << config_.to_json().dump(2) << "\n";
        } else if (cmd.rfind("profile ", 0) == 0 || cmd.rfind("use ", 0) == 0) {
            std::string pname = (cmd.rfind("profile ", 0) == 0) ? cmd.substr(8) : cmd.substr(4);
            if (switch_profile(pname)) {
                std::cout << colors::GREEN << "✔ Perfil alterado para '" << pname << "' ("
                          << backend_->backend_name() << ")" << colors::RESET << "\n";
            } else {
                std::cout << colors::RED << "✖ Perfil '" << pname << "' não encontrado em pixelforge.json\n"
                          << colors::GRAY << "  Perfis disponíveis: ollama, lmstudio, vllm, llamacpp, jan, openrouter, openai, groq, mock\n" << colors::RESET;
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

    // Suporte a flag --profile <nome> ou -p <nome>
    if (argc >= 3 && (std::string(argv[1]) == "--profile" || std::string(argv[1]) == "-p")) {
        std::string pname = argv[2];
        if (switch_profile(pname)) {
            std::cout << colors::GREEN << "[Perfil ativo: " << pname << " (" << backend_->backend_name() << ")]" << colors::RESET << "\n";
        } else {
            std::cerr << colors::RED << "Aviso: Perfil '" << pname << "' não encontrado em pixelforge.json.\n" << colors::RESET;
        }
        argc -= 2;
        argv += 2;
        if (argc <= 1) {
            run_interactive_repl();
            return 0;
        }
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

    if (subcmd == "config") {
        if (argc > 2 && std::string(argv[2]) == "init") {
            auto example = core::Config::example_json();
            std::ofstream out("pixelforge.json");
            if (out.is_open()) {
                out << example.dump(2) << "\n";
                out.close();
                std::cout << colors::GREEN << "✔ Arquivo pixelforge.json gerado no diretório atual.\n"
                          << colors::GRAY << "  Edite-o para configurar modelo, backend, limites, etc.\n" << colors::RESET;
            } else {
                std::cerr << colors::RED << "✖ Erro ao criar pixelforge.json\n" << colors::RESET;
                return 1;
            }
        } else {
            std::cout << colors::BOLD << "Configuração ativa:" << colors::RESET << "\n"
                      << config_.to_json().dump(2) << "\n";
        }
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
