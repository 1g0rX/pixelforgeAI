#pragma once

#include "third_party/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <optional>

namespace pixelforge::agent {

struct ToolCall {
    std::string id;
    std::string name;
    nlohmann::json arguments;
};

struct Message {
    std::string role; // "system", "user", "assistant", "tool"
    std::string content;
    std::string tool_call_id; // For role == "tool"
    std::vector<ToolCall> tool_calls; // For role == "assistant"

    static Message system(std::string content) {
        return Message{.role = "system", .content = std::move(content)};
    }
    static Message user(std::string content) {
        return Message{.role = "user", .content = std::move(content)};
    }
    static Message assistant(std::string content, std::vector<ToolCall> tool_calls = {}) {
        return Message{
            .role = "assistant",
            .content = std::move(content),
            .tool_calls = std::move(tool_calls)
        };
    }
    static Message tool(std::string content, std::string tool_call_id) {
        return Message{
            .role = "tool",
            .content = std::move(content),
            .tool_call_id = std::move(tool_call_id)
        };
    }
};

class Conversation {
public:
    void add_message(Message msg) { messages_.push_back(std::move(msg)); }
    const std::vector<Message>& messages() const { return messages_; }
    std::vector<Message>& messages() { return messages_; }
    void clear() { messages_.clear(); }

    /// Estima a quantidade aproximada de tokens em toda a conversa (~4 caracteres por token)
    size_t estimate_token_count() const {
        size_t total_chars = 0;
        for (const auto& m : messages_) {
            total_chars += m.role.size() + m.content.size() + m.tool_call_id.size();
            for (const auto& tc : m.tool_calls) {
                total_chars += tc.id.size() + tc.name.size() + tc.arguments.dump().size();
            }
        }
        return total_chars / 4;
    }

    /// Compacta turnos anteriores para economizar contexto
    /// Mantém o prompt de sistema e os últimos max_turns
    void compact_history(size_t max_turns = 2) {
        if (messages_.size() <= 2) return;

        Message system_msg;
        bool has_system = false;
        if (!messages_.empty() && messages_.front().role == "system") {
            system_msg = messages_.front();
            has_system = true;
        }

        // Identifica os índices das mensagens do usuário
        std::vector<size_t> user_indices;
        for (size_t i = (has_system ? 1 : 0); i < messages_.size(); ++i) {
            if (messages_[i].role == "user") {
                user_indices.push_back(i);
            }
        }

        // Se houver mais turnos do que max_turns, compacta os mais antigos
        if (user_indices.size() > max_turns) {
            size_t cutoff_turn = user_indices.size() - max_turns;
            size_t cutoff_idx = user_indices[cutoff_turn];

            std::vector<Message> new_msgs;
            if (has_system) {
                new_msgs.push_back(std::move(system_msg));
            }

            // Compacta os turnos anteriores em um resumo sintético limpo
            for (size_t t = 0; t < cutoff_turn; ++t) {
                size_t u_idx = user_indices[t];
                // Extrai apenas o prompt limpo (sem o estado ASCII antigo)
                std::string u_content = messages_[u_idx].content;
                auto prompt_pos = u_content.find("Prompt do Usuário: \"");
                if (prompt_pos != std::string::npos) {
                    auto end_pos = u_content.find("\"", prompt_pos + 19);
                    if (end_pos != std::string::npos) {
                        u_content = u_content.substr(prompt_pos, end_pos - prompt_pos + 1);
                    }
                }
                new_msgs.push_back(Message::user(u_content));

                // Procura a última mensagem do assistant correspondente a esse turno
                size_t next_u_idx = (t + 1 < user_indices.size()) ? user_indices[t + 1] : cutoff_idx;
                std::string last_thought = "Operação concluída com sucesso no sprite.";
                for (size_t k = u_idx + 1; k < next_u_idx; ++k) {
                    if (messages_[k].role == "assistant" && !messages_[k].content.empty() && messages_[k].tool_calls.empty()) {
                        last_thought = messages_[k].content;
                    }
                }
                new_msgs.push_back(Message::assistant(last_thought));
            }

            // Copia os turnos recentes na íntegra
            for (size_t i = cutoff_idx; i < messages_.size(); ++i) {
                new_msgs.push_back(std::move(messages_[i]));
            }

            messages_ = std::move(new_msgs);
        }
    }

    nlohmann::json to_openai_json() const {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& m : messages_) {
            nlohmann::json obj;
            obj["role"] = m.role;
            obj["content"] = m.content;
            if (!m.tool_call_id.empty()) {
                obj["tool_call_id"] = m.tool_call_id;
            }
            if (!m.tool_calls.empty()) {
                nlohmann::json tc_arr = nlohmann::json::array();
                for (const auto& tc : m.tool_calls) {
                    tc_arr.push_back({
                        {"id", tc.id},
                        {"type", "function"},
                        {"function", {
                            {"name", tc.name},
                            {"arguments", tc.arguments.dump()}
                        }}
                    });
                }
                obj["tool_calls"] = tc_arr;
            }
            arr.push_back(obj);
        }
        return arr;
    }

private:
    std::vector<Message> messages_;
};

} // namespace pixelforge::agent
