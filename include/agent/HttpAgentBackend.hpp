#pragma once

#include "agent/IAgentBackend.hpp"
#include <string>

namespace pixelforge::agent {

struct HttpBackendConfig {
    std::string base_url = "http://localhost:11434/v1";
    std::string api_key = "none";
    std::string model = "auto";
    double temperature = 0.2;
    int timeout_seconds = 60;
    int max_tokens = 1024;
};

class HttpAgentBackend : public IAgentBackend {
public:
    explicit HttpAgentBackend(HttpBackendConfig config = {});

    std::string backend_name() const override;

    AgentResponse generate(const Conversation& conversation, const nlohmann::json& tools_schema) override;

    const HttpBackendConfig& config() const { return config_; }
    void set_config(HttpBackendConfig config) { config_ = std::move(config); }

    std::string detect_model();

private:
    HttpBackendConfig config_;
    std::string post_json(const std::string& endpoint, const nlohmann::json& body) const;
};

} // namespace pixelforge::agent
