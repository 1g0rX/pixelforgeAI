#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace pixelforge::core {

enum class ErrorCode {
    Success = 0,
    InvalidArgument,
    OutOfBounds,
    ToolNotFound,
    PermissionDenied,
    InvalidColor,
    InvalidRegion,
    HistoryEmpty,
    IOError,
    BackendError,
    ExecutionFailed
};

class PixelForgeException : public std::runtime_error {
public:
    PixelForgeException(ErrorCode code, std::string_view message)
        : std::runtime_error(std::string(message)), code_(code) {}

    ErrorCode code() const noexcept { return code_; }

private:
    ErrorCode code_;
};

inline const char* error_code_to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success: return "Success";
        case ErrorCode::InvalidArgument: return "Invalid Argument";
        case ErrorCode::OutOfBounds: return "Out Of Bounds";
        case ErrorCode::ToolNotFound: return "Tool Not Found";
        case ErrorCode::PermissionDenied: return "Permission Denied";
        case ErrorCode::InvalidColor: return "Invalid Color";
        case ErrorCode::InvalidRegion: return "Invalid Region";
        case ErrorCode::HistoryEmpty: return "History Empty";
        case ErrorCode::IOError: return "I/O Error";
        case ErrorCode::BackendError: return "Backend Error";
        case ErrorCode::ExecutionFailed: return "Execution Failed";
        default: return "Unknown Error";
    }
}

} // namespace pixelforge::core
