#pragma once

#include "third_party/nlohmann/json.hpp"
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace pixelforge::core {

/// Configuração centralizada do PixelForge AI.
/// Carregável via arquivo JSON (pixelforge.json) com fallback para variáveis de ambiente e defaults.
struct Config {
    // --- Sprite Engine ---
    int max_sprite_width  = 512;
    int max_sprite_height = 512;
    int default_sprite_width  = 32;
    int default_sprite_height = 32;

    // --- Backend / Modelo ---
    std::string backend = "mock";           // "mock" ou "http"
    std::string api_url = "http://localhost:11434/v1";
    std::string api_key = "none";
    std::string model   = "gemma-4-e4b";
    double temperature  = 0.2;
    int timeout_seconds = 60;
    std::string active_profile = "";        // Perfil ativo opcional (ex: "ollama", "lmstudio")

    // --- Agent & Context Optimization ---
    int max_agent_iterations = 15;
    std::string context_mode = "smart";     // "smart", "minimal", "full"
    int max_history_turns = 3;              // Limite de turnos retidos para evitar saturação de tokens
    bool compact_tool_output = true;        // Resumo conciso de validação de ferramentas

    /// Carrega configuração de um arquivo JSON.
    /// Campos ausentes mantêm o valor default.
    static Config load(const std::string& filepath) {
        Config cfg;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return cfg; // Retorna defaults se o arquivo não existir
        }

        nlohmann::json j;
        try {
            // Permite comentários C/C++ (// e /* */) e trailing commas
            j = nlohmann::json::parse(file, nullptr, true, true, true);
        } catch (const std::exception& e) {
            std::cerr << "[Config] Erro ao parsear " << filepath << ": " << e.what() << "\n";
            return cfg;
        }

        // Sprite Engine
        if (j.contains("sprite")) {
            const auto& s = j["sprite"];
            if (s.contains("max_width"))       cfg.max_sprite_width  = s["max_width"].get<int>();
            if (s.contains("max_height"))      cfg.max_sprite_height = s["max_height"].get<int>();
            if (s.contains("default_width"))   cfg.default_sprite_width  = s["default_width"].get<int>();
            if (s.contains("default_height"))  cfg.default_sprite_height = s["default_height"].get<int>();
        }

        // Se houver active_profile e profiles definidos, carrega o perfil selecionado
        if (j.contains("active_profile") && j.contains("profiles")) {
            std::string active = j["active_profile"].get<std::string>();
            cfg.active_profile = active;
            if (j["profiles"].contains(active)) {
                const auto& p = j["profiles"][active];
                if (p.contains("type"))            cfg.backend = p["type"].get<std::string>();
                if (p.contains("api_url"))         cfg.api_url = p["api_url"].get<std::string>();
                if (p.contains("api_key"))         cfg.api_key = p["api_key"].get<std::string>();
                if (p.contains("model"))           cfg.model   = p["model"].get<std::string>();
                if (p.contains("temperature"))     cfg.temperature = p["temperature"].get<double>();
                if (p.contains("timeout_seconds")) cfg.timeout_seconds = p["timeout_seconds"].get<int>();
            }
        }

        // Backend direto (sobrescreve perfil se ambos existirem)
        if (j.contains("backend")) {
            const auto& b = j["backend"];
            if (b.contains("type"))            cfg.backend = b["type"].get<std::string>();
            if (b.contains("api_url"))         cfg.api_url = b["api_url"].get<std::string>();
            if (b.contains("api_key"))         cfg.api_key = b["api_key"].get<std::string>();
            if (b.contains("model"))           cfg.model   = b["model"].get<std::string>();
            if (b.contains("temperature"))     cfg.temperature = b["temperature"].get<double>();
            if (b.contains("timeout_seconds")) cfg.timeout_seconds = b["timeout_seconds"].get<int>();
        }

        // Agent
        if (j.contains("agent")) {
            const auto& a = j["agent"];
            if (a.contains("max_iterations"))    cfg.max_agent_iterations = a["max_iterations"].get<int>();
            if (a.contains("context_mode"))      cfg.context_mode = a["context_mode"].get<std::string>();
            if (a.contains("max_history_turns")) cfg.max_history_turns = a["max_history_turns"].get<int>();
            if (a.contains("compact_tool_output")) cfg.compact_tool_output = a["compact_tool_output"].get<bool>();
        }

        return cfg;
    }

    /// Aplica overrides de variáveis de ambiente (têm prioridade sobre o arquivo).
    void apply_env_overrides() {
        if (const char* v = std::getenv("PIXELFORGE_BACKEND"))    backend = v;
        if (const char* v = std::getenv("PIXELFORGE_API_URL"))    api_url = v;
        if (const char* v = std::getenv("PIXELFORGE_API_KEY"))    api_key = v;
        if (const char* v = std::getenv("PIXELFORGE_MODEL"))      model   = v;
        if (const char* v = std::getenv("PIXELFORGE_CONTEXT_MODE")) context_mode = v;
        if (const char* v = std::getenv("PIXELFORGE_MAX_SPRITE")) {
            int val = std::atoi(v);
            if (val > 0) { max_sprite_width = val; max_sprite_height = val; }
        }
    }

    /// Carrega o arquivo, depois aplica env overrides.
    /// Procura "pixelforge.json" no diretório atual e em ~/.config/pixelforge/
    static Config discover_and_load() {
        Config cfg;

        // 1. Tenta carregar do diretório atual
        if (std::filesystem::exists("pixelforge.json")) {
            cfg = load("pixelforge.json");
        }
        // 2. Tenta ~/.config/pixelforge/pixelforge.json
        else if (const char* home = std::getenv("HOME")) {
            std::string global_path = std::string(home) + "/.config/pixelforge/pixelforge.json";
            if (std::filesystem::exists(global_path)) {
                cfg = load(global_path);
            }
        }

        // 3. Env overrides sempre têm prioridade
        cfg.apply_env_overrides();

        return cfg;
    }

    /// Gera um JSON de exemplo para documentação / template
    static nlohmann::json example_json() {
        return {
            {"active_profile", "ollama"},
            {"profiles", {
                {"ollama", {
                    {"type", "http"},
                    {"api_url", "http://localhost:11434/v1"},
                    {"api_key", "none"},
                    {"model", "gemma-4-e4b"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 90}
                }},
                {"lmstudio", {
                    {"type", "http"},
                    {"api_url", "http://localhost:1234/v1"},
                    {"api_key", "none"},
                    {"model", "local-model"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 90}
                }},
                {"vllm", {
                    {"type", "http"},
                    {"api_url", "http://localhost:8000/v1"},
                    {"api_key", "none"},
                    {"model", "google/gemma-2-9b-it"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 60}
                }},
                {"llamacpp", {
                    {"type", "http"},
                    {"api_url", "http://localhost:8080/v1"},
                    {"api_key", "none"},
                    {"model", "default"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 90}
                }},
                {"jan", {
                    {"type", "http"},
                    {"api_url", "http://localhost:1337/v1"},
                    {"api_key", "none"},
                    {"model", "tinyllama-1.1b"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 90}
                }},
                {"openrouter", {
                    {"type", "http"},
                    {"api_url", "https://openrouter.ai/api/v1"},
                    {"api_key", "SUA_CHAVE_AQUI"},
                    {"model", "google/gemma-2-9b-it"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 60}
                }},
                {"openai", {
                    {"type", "http"},
                    {"api_url", "https://api.openai.com/v1"},
                    {"api_key", "SUA_CHAVE_AQUI"},
                    {"model", "gpt-4o-mini"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 60}
                }},
                {"groq", {
                    {"type", "http"},
                    {"api_url", "https://api.groq.com/openai/v1"},
                    {"api_key", "SUA_CHAVE_AQUI"},
                    {"model", "llama-3.3-70b-versatile"},
                    {"temperature", 0.2},
                    {"timeout_seconds", 45}
                }},
                {"mock", {
                    {"type", "mock"},
                    {"model", "procedural-mock"},
                    {"temperature", 0.0},
                    {"timeout_seconds", 5}
                }}
            }},
            {"sprite", {
                {"max_width", 512},
                {"max_height", 512},
                {"default_width", 32},
                {"default_height", 32}
            }},
            {"agent", {
                {"max_iterations", 15}
            }}
        };
    }

    /// Serializa a configuração atual para JSON
    nlohmann::json to_json() const {
        return {
            {"sprite", {
                {"max_width", max_sprite_width},
                {"max_height", max_sprite_height},
                {"default_width", default_sprite_width},
                {"default_height", default_sprite_height}
            }},
            {"backend", {
                {"type", backend},
                {"api_url", api_url},
                {"api_key", api_key},
                {"model", model},
                {"temperature", temperature},
                {"timeout_seconds", timeout_seconds}
            }},
            {"agent", {
                {"max_iterations", max_agent_iterations}
            }}
        };
    }
};

} // namespace pixelforge::core
