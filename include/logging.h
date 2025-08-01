#pragma once

#include "fmt/printf.h"

#include <mutex>

namespace turbokit {

enum class MessageLevel {
  MSG_NONE,
  MSG_ERROR,
  MSG_INFO,
  MSG_VERBOSE,
  MSG_DEBUG,
};

constexpr auto MSG_NONE = MessageLevel::MSG_NONE;
constexpr auto MSG_ERROR = MessageLevel::MSG_ERROR;
constexpr auto MSG_INFO = MessageLevel::MSG_INFO;
constexpr auto MSG_VERBOSE = MessageLevel::MSG_VERBOSE;
constexpr auto MSG_DEBUG = MessageLevel::MSG_DEBUG;

inline MessageLevel activeMessageLevel = MSG_INFO;

inline std::mutex messageMutex;

template <typename... Args>
[[gnu::cold]] void writeMessage(MessageLevel level, const char *format,
                                Args &&...args) {
  std::lock_guard lock(messageMutex);
  FILE *primary_output = stdout;
  FILE *secondary_output = stderr;
  if (level == MSG_ERROR) {
    std::swap(primary_output, secondary_output);
  }
  fflush(secondary_output);
  time_t current_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto *time_info = std::localtime(&current_time);
  char timestamp_buffer[0x40];
  std::strftime(timestamp_buffer, sizeof(timestamp_buffer), "%d-%m-%Y %H:%M:%S",
                time_info);
  auto message_text = fmt::sprintf(format, std::forward<Args>(args)...);
  if (!message_text.empty() && message_text.back() == '\n') {
    fmt::fprintf(primary_output, "%s turbokit: %s", timestamp_buffer,
                 message_text);
  } else {
    fmt::fprintf(primary_output, "%s turbokit: %s\n", timestamp_buffer,
                 message_text);
  }
  fflush(primary_output);
}

inline struct MessageWriter {
  template <typename... Args> void error(const char *format, Args &&...args) {
    writeMessage(MSG_ERROR, format, std::forward<Args>(args)...);
  }
  template <typename... Args> void info(const char *format, Args &&...args) {
    if (activeMessageLevel >= MSG_INFO) {
      [[unlikely]];
      writeMessage(MSG_INFO, format, std::forward<Args>(args)...);
    }
  }
  template <typename... Args> void verbose(const char *format, Args &&...args) {
    if (activeMessageLevel >= MSG_VERBOSE) {
      [[unlikely]];
      writeMessage(MSG_VERBOSE, format, std::forward<Args>(args)...);
    }
  }
  template <typename... Args> void debug(const char *format, Args &&...args) {
    if (activeMessageLevel >= MSG_DEBUG) {
      [[unlikely]];
      writeMessage(MSG_DEBUG, format, std::forward<Args>(args)...);
    }
  }
} messageWriter;

template <typename... Args>
[[noreturn]] [[gnu::cold]] void criticalError(const char *format,
                                              Args &&...args) {
  auto error_message = fmt::sprintf(format, std::forward<Args>(args)...);
  messageWriter.error(" -- TURBOKIT FATAL ERROR --\n%s\n", error_message);
  std::quick_exit(1);
}

using LogLevel = MessageLevel;
using Log = MessageWriter;
constexpr auto LOG_NONE = MSG_NONE;
constexpr auto LOG_ERROR = MSG_ERROR;
constexpr auto LOG_INFO = MSG_INFO;
constexpr auto LOG_VERBOSE = MSG_VERBOSE;
constexpr auto LOG_DEBUG = MSG_DEBUG;
inline LogLevel &currentLogLevel = activeMessageLevel;
inline std::mutex &logMutex = messageMutex;
inline Log &log = messageWriter;
template <typename... Args>
[[noreturn]] [[gnu::cold]] void fatal(const char *format, Args &&...args) {
  criticalError(format, std::forward<Args>(args)...);
}

} // namespace turbokit
