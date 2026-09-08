#include "sprite/History.hpp"
#include "core/Error.hpp"

namespace pixelforge::sprite {

History::History(size_t max_history) : max_history_(max_history) {}

void History::record_state(const Sprite& sprite, std::string action, std::string description) {
    current_step_id_++;
    undo_stack_.push_back(HistoryEntry{
        .id = current_step_id_,
        .action = std::move(action),
        .description = std::move(description),
        .snapshot = sprite,
        .is_named_checkpoint = false
    });

    if (undo_stack_.size() > max_history_) {
        undo_stack_.erase(undo_stack_.begin());
    }

    redo_stack_.clear();
}

bool History::can_undo() const {
    return !undo_stack_.empty();
}

bool History::can_redo() const {
    return !redo_stack_.empty();
}

Sprite History::undo(const Sprite& current_sprite) {
    if (!can_undo()) {
        throw core::PixelForgeException(core::ErrorCode::HistoryEmpty, "No actions to undo");
    }

    auto last_entry = std::move(undo_stack_.back());
    undo_stack_.pop_back();

    // Push current to redo stack
    redo_stack_.push_back(HistoryEntry{
        .id = last_entry.id,
        .action = last_entry.action,
        .description = last_entry.description,
        .snapshot = current_sprite,
        .is_named_checkpoint = false
    });

    return last_entry.snapshot;
}

Sprite History::redo(const Sprite& current_sprite) {
    if (!can_redo()) {
        throw core::PixelForgeException(core::ErrorCode::HistoryEmpty, "No actions to redo");
    }

    auto next_entry = std::move(redo_stack_.back());
    redo_stack_.pop_back();

    undo_stack_.push_back(HistoryEntry{
        .id = next_entry.id,
        .action = next_entry.action,
        .description = next_entry.description,
        .snapshot = current_sprite,
        .is_named_checkpoint = false
    });

    return next_entry.snapshot;
}

void History::save_checkpoint(const std::string& checkpoint_name, const Sprite& sprite) {
    checkpoints_[checkpoint_name] = sprite;
}

std::optional<Sprite> History::restore_checkpoint(const std::string& checkpoint_name) {
    auto it = checkpoints_.find(checkpoint_name);
    if (it != checkpoints_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool History::has_checkpoint(const std::string& checkpoint_name) const {
    return checkpoints_.contains(checkpoint_name);
}

std::vector<HistoryEntryInfo> History::get_log() const {
    std::vector<HistoryEntryInfo> log;
    log.reserve(undo_stack_.size());
    for (const auto& e : undo_stack_) {
        log.push_back({
            .id = e.id,
            .action = e.action,
            .description = e.description,
            .is_checkpoint = e.is_named_checkpoint
        });
    }
    return log;
}

void History::clear() {
    undo_stack_.clear();
    redo_stack_.clear();
    checkpoints_.clear();
    current_step_id_ = 0;
}

} // namespace pixelforge::sprite
