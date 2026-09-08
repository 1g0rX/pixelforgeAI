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
