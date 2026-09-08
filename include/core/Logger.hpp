#pragma once

#include <iostream>
#include <string_view>
#include <format>
#include <mutex>

namespace pixelforge::core {

enum class LogLevel {
    Debug = 0,
    Info,
    Warning,
    Error,
    None
};

class Logger {
public:
    static Logger& instance() {
        static Logger instance_;
        return instance_;
    }

    void set_level(LogLevel level) { level_ = level; }
    LogLevel level() const { return level_; }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Debug, "[DEBUG] ", fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Info, "[INFO]  ", fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Warning, "[WARN]  ", fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Error, "[ERROR] ", fmt, std::forward<Args>(args)...);
    }

private:
    Logger() = default;
    LogLevel level_ = LogLevel::Info;
    std::mutex mutex_;

    template <typename... Args>
    void log(LogLevel level, const char* prefix, std::format_string<Args...> fmt, Args&&... args) {
        if (level < level_) return;
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostream& os = (level == LogLevel::Error) ? std::cerr : std::cout;
        os << prefix << std::format(fmt, std::forward<Args>(args)...) << "\n";
    }
};

} // namespace pixelforge::core
