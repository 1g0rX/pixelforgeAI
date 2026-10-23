#include "tools/ToolRegistry.hpp"
#include "sprite/SpriteAnalyzer.hpp"
#include "core/Error.hpp"
#include "core/Logger.hpp"
#include <sstream>

namespace pixelforge::tools {

ToolRegistry::ToolRegistry() {
    // Default granted permissions
    permissions_.insert("sprite:read");
    permissions_.insert("sprite:edit");
    permissions_.insert("sprite:create");
    permissions_.insert("palette:edit");
    permissions_.insert("region:edit");
    permissions_.insert("animation:edit");
    permissions_.insert("history:edit");
}

void ToolRegistry::register_tool(std::unique_ptr<ITool> tool) {
    if (!tool) return;
    std::string name = tool->name();
    tools_[name] = std::move(tool);
}

bool ToolRegistry::has_tool(const std::string& name) const {
    return tools_.contains(name);
}

ITool* ToolRegistry::get_tool(const std::string& name) {
    auto it = tools_.find(name);
    if (it != tools_.end()) {
        return it->second.get();
    }
    return nullptr;
}

const ITool* ToolRegistry::get_tool(const std::string& name) const {
    auto it = tools_.find(name);
    if (it != tools_.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool ToolRegistry::has_permission(const std::string& perm) const {
    return permissions_.contains(perm) || permissions_.contains("*");
}

std::string ToolRegistry::validate_arguments(const ITool& tool, const nlohmann::json& args) const {
    if (!args.is_object() && !args.is_null()) {
        return "Arguments must be a JSON object";
    }

    auto schema = tool.parameter_schema();
    if (schema.contains("required") && schema["required"].is_array()) {
        for (const auto& req : schema["required"]) {
            std::string field = req.get<std::string>();
            if (!args.contains(field) || args[field].is_null()) {
                return "Missing required argument: '" + field + "'";
            }
        }
    }

    if (schema.contains("properties") && schema["properties"].is_object()) {
        const auto& props = schema["properties"];
        for (auto it = args.begin(); it != args.end(); ++it) {
            std::string key = it.key();
            if (props.contains(key) && props[key].contains("type")) {
                std::string expected_type = props[key]["type"].get<std::string>();
                const auto& val = it.value();

                if (expected_type == "integer" && !val.is_number_integer()) {
                    return "Argument '" + key + "' must be an integer";
                } else if (expected_type == "number" && !val.is_number()) {
                    return "Argument '" + key + "' must be a number";
                } else if (expected_type == "string" && !val.is_string()) {
                    return "Argument '" + key + "' must be a string";
                } else if (expected_type == "boolean" && !val.is_boolean()) {
                    return "Argument '" + key + "' must be a boolean";
                } else if (expected_type == "array" && !val.is_array()) {
                    return "Argument '" + key + "' must be an array";
                } else if (expected_type == "object" && !val.is_object()) {
                    return "Argument '" + key + "' must be an object";
                }
            }
        }
    }

    return "";
}

ToolResult ToolRegistry::execute_tool(const std::string& name, ExecutionContext& ctx, const nlohmann::json& args) {
    auto tool = get_tool(name);
    if (!tool) {
        return ToolResult::fail("Tool not found: " + name);
    }

    // Permission check
    for (const auto& req_perm : tool->required_permissions()) {
        if (!has_permission(req_perm)) {
            return ToolResult::fail("Permission denied: requires '" + req_perm + "'");
        }
    }

    // Schema validation
    std::string val_err = validate_arguments(*tool, args);
    if (!val_err.empty()) {
        return ToolResult::fail("Invalid arguments: " + val_err);
    }

    // Snapshot before modifying
    if (tool->is_modifying()) {
        ctx.history.record_state(ctx.sprite, name, args.dump());
    }

    // Execute
    ToolResult res;
    try {
        res = tool->execute(ctx, args);
    } catch (const core::PixelForgeException& e) {
        res = ToolResult::fail(std::string("Execution exception: ") + e.what());
    } catch (const std::exception& e) {
        res = ToolResult::fail(std::string("Standard exception: ") + e.what());
    }

    // Run automatic analysis
    res.analysis = sprite::SpriteAnalyzer::analyze(ctx.sprite);
    return res;
}

nlohmann::json ToolRegistry::get_tools_schema(const std::vector<std::string>& allowed_prefixes) const {
    nlohmann::json tools_array = nlohmann::json::array();
    for (const auto& [name, tool] : tools_) {
        // Se houver filtro, verifica se bate o nome do tool OU prefixo de permissão
        if (!allowed_prefixes.empty()) {
            bool matches = false;
            for (const auto& filter : allowed_prefixes) {
                if (name == filter) {
                    matches = true;
                    break;
                }
                for (const auto& perm : tool->required_permissions()) {
                    if (perm.rfind(filter, 0) == 0) {
                        matches = true;
                        break;
                    }
                }
                if (matches) break;
            }
            if (!matches) continue;
        }

        nlohmann::json func;
        func["name"] = name;
        func["description"] = tool->description();
        
        nlohmann::json params = tool->parameter_schema();
        if (!params.is_object()) {
            params = nlohmann::json::object();
        }
        if (!params.contains("type")) {
            params["type"] = "object";
        }
        if (!params.contains("properties") || !params["properties"].is_object()) {
            params["properties"] = nlohmann::json::object();
        }
        func["parameters"] = params;

        nlohmann::json entry;
        entry["type"] = "function";
        entry["function"] = func;
        tools_array.push_back(entry);
    }
    return tools_array;
}

std::string ToolRegistry::get_tools_documentation() const {
    std::ostringstream ss;
    ss << "# Available Tools (" << tools_.size() << ")\n\n";
    for (const auto& [name, tool] : tools_) {
        ss << "### `" << name << "`\n";
        ss << tool->description() << "\n\n";
        ss << "Parameters:\n```json\n" << tool->parameter_schema().dump(2) << "\n```\n\n";
    }
    return ss.str();
}

} // namespace pixelforge::tools
