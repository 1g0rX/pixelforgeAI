#include "agent/MockAgentBackend.hpp"
#include <algorithm>
#include <cctype>

namespace pixelforge::agent {

MockAgentBackend::MockAgentBackend() = default;

void MockAgentBackend::reset_state() {
    step_index_ = 0;
    custom_steps_.clear();
    plan_generated_ = false;
}

void MockAgentBackend::set_custom_plan(std::vector<std::vector<ToolCall>> steps) {
    custom_steps_ = std::move(steps);
    step_index_ = 0;
    plan_generated_ = true;
}

std::string MockAgentBackend::to_lower(std::string_view str) const {
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

void MockAgentBackend::generate_plan_for_prompt(const std::string& prompt, const std::string& current_state_text) {
    std::string p = to_lower(prompt);
    custom_steps_.clear();
    step_index_ = 0;

    // Check for correction response in conversation
    if (p.find("isolated pixel") != std::string::npos || p.find("noise") != std::string::npos) {
        custom_steps_.push_back({
            ToolCall{.id = "call_clean_1", .name = "clean_pixel_noise", .arguments = {{"threshold", 1}}}
        });
        plan_generated_ = true;
        return;
    }

    // If sprite is empty or blank, create base character first before animating!
    bool is_blank = (current_state_text.find("canvas is blank") != std::string::npos ||
                     current_state_text.find("Colors: 0") != std::string::npos);

    // 1. Animation prompts (e.g. "pulo com 6 frames", "caminhada", "walk animation")
    if (p.find("anima") != std::string::npos || p.find("frame") != std::string::npos ||
        p.find("pular") != std::string::npos || p.find("pulo") != std::string::npos ||
        p.find("caminha") != std::string::npos || p.find("walk") != std::string::npos) {

        if (is_blank) {
            // Draw base slime character first
            custom_steps_.push_back({
                ToolCall{.id = "call_sl_create_base", .name = "create_sprite", .arguments = {{"width", 32}, {"height", 32}, {"name", "slime"}}},
                ToolCall{.id = "call_sl_palette_base", .name = "set_palette", .arguments = {
                    {"colors", nlohmann::json::array({
                        "#000000", "#144414", "#267A26", "#42B842", "#84E884", "#FFFFFF", "#1E1E1E"
                    })}
                }},
                ToolCall{.id = "call_sl_body_reg_base", .name = "create_region", .arguments = {{"name", "body"}, {"x", 8}, {"y", 12}, {"width", 16}, {"height", 13}}},
                ToolCall{.id = "call_sl_body_poly_base", .name = "draw_polygon", .arguments = {
                    {"points", nlohmann::json::array({
                        nlohmann::json::array({15, 12}),
                        nlohmann::json::array({17, 12}),
                        nlohmann::json::array({23, 16}),
                        nlohmann::json::array({24, 22}),
                        nlohmann::json::array({22, 24}),
                        nlohmann::json::array({9, 24}),
                        nlohmann::json::array({7, 22}),
                        nlohmann::json::array({8, 16})
                    })},
                    {"color", "#42B842"},
                    {"fill", true}
                }},
                ToolCall{.id = "call_sl_outline_base", .name = "create_outline", .arguments = {{"color", "#144414"}}},
                ToolCall{.id = "call_sl_eye1_base", .name = "set_pixel", .arguments = {{"x", 13}, {"y", 17}, {"color", "#1E1E1E"}}},
                ToolCall{.id = "call_sl_eye2_base", .name = "set_pixel", .arguments = {{"x", 19}, {"y", 17}, {"color", "#1E1E1E"}}}
            });
        }

        int frame_count = 6;
        if (p.find("4 frame") != std::string::npos) frame_count = 4;
        else if (p.find("8 frame") != std::string::npos) frame_count = 8;

        std::string anim_type = (p.find("pulo") != std::string::npos || p.find("pular") != std::string::npos) ? "jump" : "walk";

        // Step 0: Initialize animation
        custom_steps_.push_back({
            ToolCall{.id = "call_anim_init", .name = "create_animation", .arguments = {{"name", anim_type}, {"duration_ms", 120}}}
        });

        if (anim_type == "jump") {
            // Frame 1: Anticipation / squash
            custom_steps_.push_back({
                ToolCall{.id = "call_anim_f1_dup", .name = "duplicate_frame", .arguments = {{"frame_index", 0}}},
                ToolCall{.id = "call_anim_f1_load", .name = "get_frame", .arguments = {{"frame_index", 1}}},
                ToolCall{.id = "call_anim_f1_squash", .name = "move_region", .arguments = {{"region", "body"}, {"dx", 0}, {"dy", 2}}},
                ToolCall{.id = "call_anim_f1_save", .name = "update_frame", .arguments = {{"frame_index", 1}}}
            });

            // Frame 2: Upward stretch
            custom_steps_.push_back({
                ToolCall{.id = "call_anim_f2_dup", .name = "duplicate_frame", .arguments = {{"frame_index", 0}}},
                ToolCall{.id = "call_anim_f2_load", .name = "get_frame", .arguments = {{"frame_index", 2}}},
                ToolCall{.id = "call_anim_f2_stretch", .name = "move_region", .arguments = {{"region", "body"}, {"dx", 0}, {"dy", -4}}},
                ToolCall{.id = "call_anim_f2_save", .name = "update_frame", .arguments = {{"frame_index", 2}}}
            });

            // Frame 3: Apex jump
            custom_steps_.push_back({
                ToolCall{.id = "call_anim_f3_dup", .name = "duplicate_frame", .arguments = {{"frame_index", 0}}},
                ToolCall{.id = "call_anim_f3_load", .name = "get_frame", .arguments = {{"frame_index", 3}}},
                ToolCall{.id = "call_anim_f3_apex", .name = "move_region", .arguments = {{"region", "body"}, {"dx", 0}, {"dy", -6}}},
                ToolCall{.id = "call_anim_f3_save", .name = "update_frame", .arguments = {{"frame_index", 3}}}
            });

            // Frame 4: Falling down
            custom_steps_.push_back({
                ToolCall{.id = "call_anim_f4_dup", .name = "duplicate_frame", .arguments = {{"frame_index", 0}}},
                ToolCall{.id = "call_anim_f4_load", .name = "get_frame", .arguments = {{"frame_index", 4}}},
                ToolCall{.id = "call_anim_f4_fall", .name = "move_region", .arguments = {{"region", "body"}, {"dx", 0}, {"dy", -2}}},
                ToolCall{.id = "call_anim_f4_save", .name = "update_frame", .arguments = {{"frame_index", 4}}}
            });

            // Frame 5: Impact / recovery
            custom_steps_.push_back({
                ToolCall{.id = "call_anim_f5_dup", .name = "duplicate_frame", .arguments = {{"frame_index", 0}}},
                ToolCall{.id = "call_anim_f5_load", .name = "get_frame", .arguments = {{"frame_index", 5}}},
                ToolCall{.id = "call_anim_f5_land", .name = "move_region", .arguments = {{"region", "body"}, {"dx", 0}, {"dy", 1}}},
                ToolCall{.id = "call_anim_f5_save", .name = "update_frame", .arguments = {{"frame_index", 5}}}
            });
        } else {
            // Walk cycle
            for (int f = 1; f < frame_count; ++f) {
                int leg_dx = (f % 2 == 1) ? 2 : -2;
                custom_steps_.push_back({
                    ToolCall{.id = "call_walk_dup_" + std::to_string(f), .name = "duplicate_frame", .arguments = {{"frame_index", 0}}},
                    ToolCall{.id = "call_walk_load_" + std::to_string(f), .name = "get_frame", .arguments = {{"frame_index", f}}},
                    ToolCall{.id = "call_walk_leg_" + std::to_string(f), .name = "move_region", .arguments = {{"region", "left_leg"}, {"dx", leg_dx}, {"dy", 0}}},
                    ToolCall{.id = "call_walk_arm_" + std::to_string(f), .name = "move_region", .arguments = {{"region", "right_arm"}, {"dx", -leg_dx}, {"dy", 0}}},
                    ToolCall{.id = "call_walk_save_" + std::to_string(f), .name = "update_frame", .arguments = {{"frame_index", f}}}
                });
            }
        }

        // Compare keyframes for verification
        custom_steps_.push_back({
            ToolCall{.id = "call_anim_cmp", .name = "compare_frames", .arguments = {{"frame_a", 0}, {"frame_b", 3}}}
        });

        plan_generated_ = true;
        return;
    }

    // 2. Palette modification (e.g. "mude a paleta para tons de azul")
    if (p.find("azul") != std::string::npos || p.find("blue") != std::string::npos) {
        custom_steps_.push_back({
            ToolCall{.id = "call_pal_blue", .name = "set_palette", .arguments = {
                {"colors", nlohmann::json::array({
                    "#000000", "#101D38", "#1E3C72", "#2A5298", "#4A90E2", "#87CEEB", "#E0F7FA", "#FFFFFF"
                })}
            }},
            ToolCall{.id = "call_rep_green1", .name = "replace_color", .arguments = {{"old_color", "#42B842"}, {"new_color", "#4A90E2"}}},
            ToolCall{.id = "call_rep_green2", .name = "replace_color", .arguments = {{"old_color", "#267A26"}, {"new_color", "#2A5298"}}},
            ToolCall{.id = "call_rep_green3", .name = "replace_color", .arguments = {{"old_color", "#84E884"}, {"new_color", "#87CEEB"}}},
            ToolCall{.id = "call_rep_green4", .name = "replace_color", .arguments = {{"old_color", "#144414"}, {"new_color", "#101D38"}}}
        });
        plan_generated_ = true;
        return;
    }

    // 3. Add sword / weapon (e.g. "adicione uma espada na mão direita")
    if (p.find("espada") != std::string::npos || p.find("sword") != std::string::npos) {
        custom_steps_.push_back({
            ToolCall{.id = "call_reg_sword", .name = "create_region", .arguments = {{"name", "weapon"}, {"x", 22}, {"y", 10}, {"width", 4}, {"height", 14}}},
            ToolCall{.id = "call_draw_blade", .name = "draw_line", .arguments = {{"x0", 23}, {"y0", 11}, {"x1", 23}, {"y1", 20}, {"color", "#C2C3C7"}}},
            ToolCall{.id = "call_draw_guard", .name = "draw_line", .arguments = {{"x0", 21}, {"y0", 20}, {"x1", 25}, {"y1", 20}, {"color", "#FFA300"}}},
            ToolCall{.id = "call_draw_hilt", .name = "draw_line", .arguments = {{"x0", 23}, {"y0", 21}, {"x1", 23}, {"y1", 23}, {"color", "#AB5236"}}},
            ToolCall{.id = "call_outline_sword", .name = "create_outline", .arguments = {{"color", "#000000"}, {"region", "weapon"}}}
        });
        plan_generated_ = true;
        return;
    }

    // 4. Knight prompt (e.g. "Crie um cavaleiro medieval 32x32")
    if (p.find("cavaleiro") != std::string::npos || p.find("knight") != std::string::npos) {
        // Step 1: Canvas & Palette
        custom_steps_.push_back({
            ToolCall{.id = "call_kn_create", .name = "create_sprite", .arguments = {{"width", 32}, {"height", 32}, {"name", "knight"}}},
            ToolCall{.id = "call_kn_palette", .name = "set_palette", .arguments = {{"preset", "dawnbringer16"}}}
        });

        // Step 2: Head / Helmet & Regions
        custom_steps_.push_back({
            ToolCall{.id = "call_kn_reg_head", .name = "create_region", .arguments = {{"name", "head"}, {"x", 11}, {"y", 6}, {"width", 10}, {"height", 8}}},
            ToolCall{.id = "call_kn_draw_head", .name = "draw_rectangle", .arguments = {{"x", 12}, {"y", 7}, {"width", 8}, {"height", 7}, {"color", "#8595A1"}, {"fill", true}}},
            ToolCall{.id = "call_kn_visor", .name = "draw_line", .arguments = {{"x0", 14}, {"y0", 10}, {"x1", 18}, {"y1", 10}, {"color", "#140C1C"}}}
        });

        // Step 3: Body & Armor
        custom_steps_.push_back({
            ToolCall{.id = "call_kn_reg_body", .name = "create_region", .arguments = {{"name", "body"}, {"x", 10}, {"y", 14}, {"width", 12}, {"height", 9}}},
            ToolCall{.id = "call_kn_draw_body", .name = "draw_rectangle", .arguments = {{"x", 11}, {"y", 14}, {"width", 10}, {"height", 9}, {"color", "#597DCE"}, {"fill", true}}},
            ToolCall{.id = "call_kn_belt", .name = "draw_line", .arguments = {{"x0", 11}, {"y0", 22}, {"x1", 20}, {"y1", 22}, {"color", "#854C30"}}}
        });

        // Step 4: Arms & Legs
        custom_steps_.push_back({
            ToolCall{.id = "call_kn_reg_lleg", .name = "create_region", .arguments = {{"name", "left_leg"}, {"x", 12}, {"y", 23}, {"width", 3}, {"height", 6}}},
            ToolCall{.id = "call_kn_draw_lleg", .name = "draw_rectangle", .arguments = {{"x", 12}, {"y", 23}, {"width", 3}, {"height", 6}, {"color", "#4E4A4E"}, {"fill", true}}},
            ToolCall{.id = "call_kn_reg_rleg", .name = "create_region", .arguments = {{"name", "right_leg"}, {"x", 17}, {"y", 23}, {"width", 3}, {"height", 6}}},
            ToolCall{.id = "call_kn_draw_rleg", .name = "draw_rectangle", .arguments = {{"x", 17}, {"y", 23}, {"width", 3}, {"height", 6}, {"color", "#4E4A4E"}, {"fill", true}}},
            ToolCall{.id = "call_kn_reg_larm", .name = "create_region", .arguments = {{"name", "left_arm"}, {"x", 8}, {"y", 15}, {"width", 3}, {"height", 7}}},
            ToolCall{.id = "call_kn_draw_larm", .name = "draw_rectangle", .arguments = {{"x", 8}, {"y", 15}, {"width", 3}, {"height", 7}, {"color", "#8595A1"}, {"fill", true}}},
            ToolCall{.id = "call_kn_reg_rarm", .name = "create_region", .arguments = {{"name", "right_arm"}, {"x", 21}, {"y", 15}, {"width", 3}, {"height", 7}}},
            ToolCall{.id = "call_kn_draw_rarm", .name = "draw_rectangle", .arguments = {{"x", 21}, {"y", 15}, {"width", 3}, {"height", 7}, {"color", "#8595A1"}, {"fill", true}}}
        });

        // Step 5: Sword & Shield
        custom_steps_.push_back({
            ToolCall{.id = "call_kn_reg_wep", .name = "create_region", .arguments = {{"name", "weapon"}, {"x", 23}, {"y", 11}, {"width", 4}, {"height", 13}}},
            ToolCall{.id = "call_kn_sword_b", .name = "draw_line", .arguments = {{"x0", 24}, {"y0", 11}, {"x1", 24}, {"y1", 20}, {"color", "#DEEED6"}}},
            ToolCall{.id = "call_kn_sword_h", .name = "draw_line", .arguments = {{"x0", 23}, {"y0", 20}, {"x1", 25}, {"y1", 20}, {"color", "#DAD45E"}}},
            ToolCall{.id = "call_kn_reg_shld", .name = "create_region", .arguments = {{"name", "shield"}, {"x", 6}, {"y", 15}, {"width", 5}, {"height", 8}}},
            ToolCall{.id = "call_kn_shield", .name = "draw_rectangle", .arguments = {{"x", 6}, {"y", 15}, {"width", 5}, {"height", 8}, {"color", "#D04648"}, {"fill", true}}}
        });

        // Step 6: Outline & Polish
        custom_steps_.push_back({
            ToolCall{.id = "call_kn_outline", .name = "create_outline", .arguments = {{"color", "#140C1C"}}},
            ToolCall{.id = "call_kn_shadow", .name = "add_shadow", .arguments = {{"dx", 1}, {"dy", 1}, {"color", "#140C1C"}}}
        });

        plan_generated_ = true;
        return;
    }

    // 5. Slime prompt (e.g. "Crie um slime verde 32x32 com no máximo 8 cores") - Section 24 Default Goal
    // Step 1: Create canvas & palette
    custom_steps_.push_back({
        ToolCall{.id = "call_sl_create", .name = "create_sprite", .arguments = {{"width", 32}, {"height", 32}, {"name", "slime"}}},
        ToolCall{.id = "call_sl_palette", .name = "set_palette", .arguments = {
            {"colors", nlohmann::json::array({
                "#000000", "#144414", "#267A26", "#42B842", "#84E884", "#FFFFFF", "#1E1E1E"
            })}
        }}
    });

    // Step 2: Body shape and region
    custom_steps_.push_back({
        ToolCall{.id = "call_sl_body_reg", .name = "create_region", .arguments = {{"name", "body"}, {"x", 8}, {"y", 11}, {"width", 16}, {"height", 14}}},
        ToolCall{.id = "call_sl_body_poly", .name = "draw_polygon", .arguments = {
            {"points", nlohmann::json::array({
                nlohmann::json::array({15, 11}),
                nlohmann::json::array({17, 11}),
                nlohmann::json::array({23, 16}),
                nlohmann::json::array({24, 22}),
                nlohmann::json::array({22, 24}),
                nlohmann::json::array({9, 24}),
                nlohmann::json::array({7, 22}),
                nlohmann::json::array({8, 16})
            })},
            {"color", "#42B842"},
            {"fill", true}
        }},
        ToolCall{.id = "call_sl_fill", .name = "fill_region", .arguments = {{"x", 16}, {"y", 18}, {"color", "#42B842"}}}
    });

    // Step 3: Eyes & Facial Expression
    custom_steps_.push_back({
        ToolCall{.id = "call_sl_eyes_reg", .name = "create_region", .arguments = {{"name", "eyes"}, {"x", 11}, {"y", 16}, {"width", 10}, {"height", 4}}},
        ToolCall{.id = "call_sl_eye_l1", .name = "set_pixel", .arguments = {{"x", 12}, {"y", 17}, {"color", "#1E1E1E"}}},
        ToolCall{.id = "call_sl_eye_l2", .name = "set_pixel", .arguments = {{"x", 12}, {"y", 18}, {"color", "#1E1E1E"}}},
        ToolCall{.id = "call_sl_eye_l3", .name = "set_pixel", .arguments = {{"x", 13}, {"y", 17}, {"color", "#1E1E1E"}}},
        ToolCall{.id = "call_sl_eye_l4", .name = "set_pixel", .arguments = {{"x", 12}, {"y", 16}, {"color", "#FFFFFF"}}},
        ToolCall{.id = "call_sl_eye_r1", .name = "set_pixel", .arguments = {{"x", 18}, {"y", 17}, {"color", "#1E1E1E"}}},
        ToolCall{.id = "call_sl_eye_r2", .name = "set_pixel", .arguments = {{"x", 18}, {"y", 18}, {"color", "#1E1E1E"}}},
        ToolCall{.id = "call_sl_eye_r3", .name = "set_pixel", .arguments = {{"x", 19}, {"y", 17}, {"color", "#1E1E1E"}}},
        ToolCall{.id = "call_sl_eye_r4", .name = "set_pixel", .arguments = {{"x", 18}, {"y", 16}, {"color", "#FFFFFF"}}}
    });

    // Step 4: Outline, Highlights and Shading
    custom_steps_.push_back({
        ToolCall{.id = "call_sl_outline", .name = "create_outline", .arguments = {{"color", "#144414"}}},
        ToolCall{.id = "call_sl_highlight", .name = "add_highlight", .arguments = {{"dx", -1}, {"dy", -1}, {"color", "#84E884"}}},
        ToolCall{.id = "call_sl_shadow", .name = "add_shadow", .arguments = {{"dx", 1}, {"dy", 1}, {"color", "#144414"}}}
    });

    plan_generated_ = true;
}

AgentResponse MockAgentBackend::generate(const Conversation& conversation, const nlohmann::json&) {
    const auto& msgs = conversation.messages();
    if (msgs.empty()) {
        return AgentResponse{.content = "No messages to process", .is_finished = true};
    }

    std::string initial_user_msg;
    std::string latest_critique;
    for (const auto& m : msgs) {
        if (m.role == "user") {
            if (initial_user_msg.empty()) {
                initial_user_msg = m.content;
            } else {
                latest_critique = m.content;
            }
        }
    }

    if (!latest_critique.empty() && (latest_critique.find("ruído") != std::string::npos || latest_critique.find("isolados") != std::string::npos)) {
        return AgentResponse{
            .content = "Detectei pixel isolado no resultado da avaliação. Executando limpeza com clean_pixel_noise.",
            .tool_calls = {
                ToolCall{.id = "call_auto_clean", .name = "clean_pixel_noise", .arguments = {{"threshold", 1}}}
            },
            .is_finished = false
        };
    }

    // Extract prompt and state from initial user message
    std::string prompt = initial_user_msg;
    auto q1 = initial_user_msg.find('"');
    auto q2 = initial_user_msg.find('"', q1 + 1);
    if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
        prompt = initial_user_msg.substr(q1 + 1, q2 - q1 - 1);
    }
    std::string current_state = initial_user_msg;

    if (!plan_generated_) {
        generate_plan_for_prompt(prompt, current_state);
    }

    if (step_index_ < static_cast<int>(custom_steps_.size())) {
        auto calls = custom_steps_[static_cast<size_t>(step_index_)];
        step_index_++;
        std::string explanation = "Executando etapa " + std::to_string(step_index_) +
                                  " de " + std::to_string(custom_steps_.size()) + "...";
        return AgentResponse{
            .content = explanation,
            .tool_calls = std::move(calls),
            .is_finished = false
        };
    }

    // All steps executed! Finished.
    plan_generated_ = false;
    step_index_ = 0;
    return AgentResponse{
        .content = "Planejamento e execução concluídos com sucesso! O sprite atende aos critérios estéticos e estruturais solicitados.",
        .tool_calls = {},
        .is_finished = true
    };
}

} // namespace pixelforge::agent
