#include "agent/HttpAgentBackend.hpp"
#include "core/Error.hpp"
#include "core/Logger.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>

namespace pixelforge::agent {

HttpAgentBackend::HttpAgentBackend(HttpBackendConfig config)
    : config_(std::move(config)) {
    // Overrides de ambiente são aplicados pelo core::Config antes de chegar aqui.
    // Se instanciado diretamente (sem Config), ainda suporta env overrides:
    if (const char* env_url = std::getenv("PIXELFORGE_API_URL")) {
        config_.base_url = env_url;
    }
    if (const char* env_key = std::getenv("PIXELFORGE_API_KEY")) {
        config_.api_key = env_key;
    }
    if (const char* env_model = std::getenv("PIXELFORGE_MODEL")) {
        config_.model = env_model;
    }

    // Se o modelo for "auto", vazio ou "default", detecta o que estiver carregado
    if (config_.model.empty() || config_.model == "auto" || config_.model == "default") {
        std::string detected = detect_model();
        if (!detected.empty()) {
            config_.model = detected;
        }
    }
}

std::string HttpAgentBackend::detect_model() {
    std::string endpoint = config_.base_url;
    if (endpoint.back() != '/') endpoint += "/";
    endpoint += "models";

    std::string cmd = "curl -sS -m 5 -X GET \"" + endpoint + "\" ";
    if (!config_.api_key.empty() && config_.api_key != "none") {
        cmd += "-H \"Authorization: Bearer " + config_.api_key + "\" ";
    }
    cmd += "2>/dev/null";

    std::string response;
    FILE* fp = popen(cmd.c_str(), "r");
    if (fp) {
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            response += buffer;
        }
        pclose(fp);
    }

    if (!response.empty()) {
        try {
            auto j = nlohmann::json::parse(response);
            if (j.contains("data") && j["data"].is_array() && !j["data"].empty()) {
                // Prioriza modelos de chat/instrução (ignora embeddings como sentence-transformer, nomic-embed, etc.)
                for (const auto& m : j["data"]) {
                    std::string mid = m.value("id", "");
                    std::string lower = mid;
                    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
                    if (!mid.empty() &&
                        lower.find("embed") == std::string::npos &&
                        lower.find("transformer") == std::string::npos &&
                        lower.find("sentence") == std::string::npos &&
                        lower.find("bge") == std::string::npos &&
                        lower.find("bert") == std::string::npos) {
                        return mid;
                    }
                }
            }
        } catch (...) {}
    }
    return "";
}

std::string HttpAgentBackend::backend_name() const {
    std::string m = config_.model;
    if (m.empty() || m == "auto" || m == "default") {
        m = const_cast<HttpAgentBackend*>(this)->detect_model();
        if (m.empty()) m = "auto (servidor offline ou nenhum carregado)";
    }
    return "HttpAgent (" + m + " @ " + config_.base_url + ")";
}

std::string HttpAgentBackend::post_json(const std::string& endpoint, const nlohmann::json& body) const {
    // Write temporary request payload
    std::string temp_req = "/tmp/pixelforge_req_" + std::to_string(std::rand()) + ".json";
    std::string err_file = "/tmp/pixelforge_err_" + std::to_string(std::rand()) + ".txt";
    {
        std::ofstream out(temp_req);
        if (!out.is_open()) {
            throw core::PixelForgeException(core::ErrorCode::IOError, "Failed to write temp request file");
        }
        out << body.dump();
    }
    // Also save last request for debugging
    {
        std::ofstream debug_out("/tmp/last_pixelforge_req.json");
        if (debug_out.is_open()) {
            debug_out << body.dump(2);
        }
    }

    std::string cmd = "curl -sS -m " + std::to_string(config_.timeout_seconds) +
                      " -X POST \"" + endpoint + "\" " +
                      "-H \"Content-Type: application/json\" ";
    if (!config_.api_key.empty() && config_.api_key != "none") {
        cmd += "-H \"Authorization: Bearer " + config_.api_key + "\" ";
    }
    cmd += "-d @" + temp_req + " 2> " + err_file;

    std::string response;
    FILE* fp = popen(cmd.c_str(), "r");
    if (fp) {
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            response += buffer;
        }
        pclose(fp);
    }
    std::filesystem::remove(temp_req);

    if (response.empty()) {
        std::string err_msg;
        std::ifstream err_in(err_file);
        if (err_in.is_open()) {
            std::stringstream ss;
            ss << err_in.rdbuf();
            err_msg = ss.str();
        }
        std::filesystem::remove(err_file);
        throw core::PixelForgeException(core::ErrorCode::BackendError,
            "Empty or failed response from backend server at " + endpoint +
            (err_msg.empty() ? "" : ("\nCurl details: " + err_msg)));
    }
    std::filesystem::remove(err_file);
    return response;
}

AgentResponse HttpAgentBackend::generate(const Conversation& conversation, const nlohmann::json& tools_schema) {
    // Se o modelo for "auto", vazio ou "default", detecta agora
    if (config_.model.empty() || config_.model == "auto" || config_.model == "default") {
        std::string detected = detect_model();
        if (!detected.empty()) {
            config_.model = detected;
        } else {
            throw core::PixelForgeException(core::ErrorCode::BackendError,
                "Nenhum modelo foi especificado e não foi possível detectar automaticamente nenhum modelo ativo em " +
                config_.base_url + ".\nCertifique-se de que o servidor local (LM Studio / Ollama) está aberto e com um modelo carregado.");
        }
    }

    std::string endpoint = config_.base_url;
    if (endpoint.back() != '/') endpoint += "/";
    endpoint += "chat/completions";

    nlohmann::json req_body;
    req_body["model"] = config_.model;
    req_body["messages"] = conversation.to_openai_json();
    req_body["temperature"] = config_.temperature;
    req_body["max_tokens"] = config_.max_tokens;
    if (!tools_schema.empty()) {
        req_body["tools"] = tools_schema;
        req_body["tool_choice"] = "auto";
    }

    std::string raw_resp = post_json(endpoint, req_body);
    nlohmann::json resp_json;
    try {
        resp_json = nlohmann::json::parse(raw_resp);
    } catch (const std::exception& e) {
        throw core::PixelForgeException(core::ErrorCode::BackendError, "Failed to parse JSON response: " + std::string(e.what()) + "\nRaw: " + raw_resp);
    }

    if (resp_json.contains("error")) {
        std::string err_msg = resp_json["error"].dump();
        throw core::PixelForgeException(core::ErrorCode::BackendError, "API Error: " + err_msg);
    }

    if (!resp_json.contains("choices") || resp_json["choices"].empty()) {
        throw core::PixelForgeException(core::ErrorCode::BackendError, "No choices in model response");
    }

    const auto& choice = resp_json["choices"][0];
    const auto& msg = choice["message"];

    AgentResponse agent_resp;
    if (msg.contains("content") && !msg["content"].is_null()) {
        agent_resp.content = msg["content"].get<std::string>();
    }

    if (msg.contains("tool_calls") && msg["tool_calls"].is_array()) {
        for (const auto& tc : msg["tool_calls"]) {
            ToolCall call;
            call.id = tc.value("id", "call_default");
            if (tc.contains("function")) {
                const auto& fn = tc["function"];
                call.name = fn.value("name", "");
                if (fn.contains("arguments")) {
                    if (fn["arguments"].is_string()) {
                        try {
                            call.arguments = nlohmann::json::parse(fn["arguments"].get<std::string>());
                        } catch (...) {
                            call.arguments = nlohmann::json::object();
                        }
                    } else if (fn["arguments"].is_object()) {
                        call.arguments = fn["arguments"];
                    }
                }
            }
            if (!call.name.empty()) {
                agent_resp.tool_calls.push_back(std::move(call));
            }
        }
    }

    agent_resp.is_finished = agent_resp.tool_calls.empty();
    return agent_resp;
}

} // namespace pixelforge::agent
