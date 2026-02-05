#ifndef INC_2A03_LOG_H
#define INC_2A03_LOG_H

#include <iostream>
#include <string>
#include <unordered_set>

namespace NES {

#ifdef NES_ENABLE_LOGGING

/// Debug logging facility with named handles that can be individually enabled/disabled
class Log {
   public:
    /// Get the singleton instance
    static Log &instance() {
        static Log log;
        return log;
    }

    /// Enable logging for a specific handle
    void enable(const std::string &handle) { enabled_handles.insert(handle); }

    /// Disable logging for a specific handle
    void disable(const std::string &handle) { enabled_handles.erase(handle); }

    /// Check if a handle is enabled
    bool is_enabled(const std::string &handle) const {
        return enabled_handles.find(handle) != enabled_handles.end();
    }

    /// Set the output stream (defaults to std::cerr)
    void set_output(std::ostream *out) { output = out; }

    /// Get the output stream
    std::ostream *get_output() const { return output; }

    /// Returns a stream for logging with handle prefix
    std::ostream &stream(const std::string &handle) {
        *output << handle << ": ";
        return *output;
    }

    /// Returns a stream (null if disabled) - for use as expression/function argument
    std::ostream &stream_expr(const std::string &handle) {
        if (!is_enabled(handle)) {
            return null_stream;
        }
        *output << handle << ": ";
        return *output;
    }

   private:
    Log() : output(&std::cerr) {}
    Log(const Log &) = delete;
    Log &operator=(const Log &) = delete;

    std::unordered_set<std::string> enabled_handles;
    std::ostream *output;

    // Null stream that discards output
    class NullBuffer : public std::streambuf {
       public:
        int overflow(int c) override { return c; }
    };
    NullBuffer null_buffer;
    std::ostream null_stream{&null_buffer};
};

/// Short-circuit macro: if handle is disabled, the entire expression after
/// NES_LOG() is never evaluated (including std::format calls, etc.)
/// Use for statement-style logging: NES_LOG("X") << "message" << std::endl;
#define NES_LOG(handle) \
    if (!NES::Log::instance().is_enabled(handle)) {} \
    else NES::Log::instance().stream(handle)

/// Expression macro: returns a stream reference (null stream if disabled).
/// Use when passing to functions: print_data(NES_LOG_STREAM("X"));
/// Note: Arguments are still evaluated even when disabled.
#define NES_LOG_STREAM(handle) NES::Log::instance().stream_expr(handle)

#define NES_LOG_ENABLED(handle) NES::Log::instance().is_enabled(handle)

#else  // NES_ENABLE_LOGGING not defined - compile out all logging

/// Stub Log class when logging is disabled
class Log {
   public:
    static Log &instance() {
        static Log log;
        return log;
    }
    void enable(const std::string &) {}
    void disable(const std::string &) {}
    bool is_enabled(const std::string &) const { return false; }
    void set_output(std::ostream *) {}
    std::ostream *get_output() const { return nullptr; }
};

/// if constexpr (false) guarantees the discarded statement is never compiled.
/// The entire expression after NES_LOG(), including std::format calls, is removed.
#define NES_LOG(handle) if constexpr (false) std::cerr

#define NES_LOG_STREAM(handle) std::cerr
#define NES_LOG_ENABLED(handle) false

#endif  // NES_ENABLE_LOGGING

}  // namespace NES

#endif  // INC_2A03_LOG_H
