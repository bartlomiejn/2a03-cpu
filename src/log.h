#ifndef INC_2A03_LOG_H
#define INC_2A03_LOG_H

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace NES {

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

    /// Log a message with a handle prefix
    template <typename... Args>
    void log(const std::string &handle, Args &&...args) {
        if (!is_enabled(handle) || !output) return;
        *output << handle << ": ";
        ((*output << std::forward<Args>(args)), ...);
    }

    /// Returns a stream for logging (for use with manipulators like std::hex)
    /// Returns a null stream if handle is disabled
    std::ostream &stream(const std::string &handle) {
        if (!is_enabled(handle) || !output) {
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

#define NES_LOG(handle) NES::Log::instance().stream(handle)

#define NES_LOG_ENABLED(handle) NES::Log::instance().is_enabled(handle)

}  // namespace NES

#endif  // INC_2A03_LOG_H
