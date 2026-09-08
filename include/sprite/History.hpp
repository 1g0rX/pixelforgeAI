#pragma once

#include "sprite/Sprite.hpp"
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace pixelforge::sprite {

struct HistoryEntry {
    size_t id;
    std::string action;
    std::string description;
    Sprite snapshot;
    bool is_named_checkpoint = false;
};

struct HistoryEntryInfo {
    size_t id;
    std::string action;
    std::string description;
    bool is_checkpoint;
};

class History {
public:
    explicit History(size_t max_history = 50);

    void record_state(const Sprite& sprite, std::string action, std::string description = "");
    bool can_undo() const;
    bool can_redo() const;

    Sprite undo(const Sprite& current_sprite);
    Sprite redo(const Sprite& current_sprite);

    void save_checkpoint(const std::string& checkpoint_name, const Sprite& sprite);
    std::optional<Sprite> restore_checkpoint(const std::string& checkpoint_name);
    bool has_checkpoint(const std::string& checkpoint_name) const;

    std::vector<HistoryEntryInfo> get_log() const;
    void clear();

    size_t current_step() const { return current_step_id_; }

private:
    size_t max_history_;
    size_t current_step_id_ = 0;
    std::vector<HistoryEntry> undo_stack_;
    std::vector<HistoryEntry> redo_stack_;
    std::unordered_map<std::string, Sprite> checkpoints_;
};

} // namespace pixelforge::sprite
