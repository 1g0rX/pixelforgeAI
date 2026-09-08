#pragma once

#include "agent/IAgentBackend.hpp"
#include <string>

namespace pixelforge::agent {

struct HttpBackendConfig {
    std::string base_url = "http://localhost:11434/v1";
    std::string api_key = "none";
    std::string model = "gemma-4-e4b";
    float temperature = 0.2f;
    int timeout_seconds = 60;
};

class HttpAgentBackend : public IAgentBackend {
public:
    explicit HttpAgentBackend(HttpBackendConfig config = {});

    std::string backend_name() const override {
        return "HttpAgent (" + config_.model + " @ " + config_.base_url + ")";
    }

    AgentResponse generate(const Conversation& conversation, const nlohmann::json& tools_schema) override;

    const HttpBackendConfig& config() const { return config_; }
    void set_config(HttpBackendConfig config) { config_ = std::move(config); }

private:
    HttpBackendConfig config_;
    std::string post_json(const std::string& endpoint, const nlohmann::json& body) const;
};

} // namespace pixelforge::agent
