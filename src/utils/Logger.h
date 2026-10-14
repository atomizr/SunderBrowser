#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// SPDLOG_LOGGER_* передаёт source_loc (__FILE__, __LINE__),
// поэтому в pattern [%s:%#] будет корректный файл/строка.
#define LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(Logger::instance(), __VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(Logger::instance(), __VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_LOGGER_INFO(Logger::instance(), __VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_LOGGER_WARN(Logger::instance(), __VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(Logger::instance(), __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Logger::instance(), __VA_ARGS__)

#define LOG_DEBUG_ONLY(...)  SPDLOG_LOGGER_DEBUG(Logger::debugLogger(), __VA_ARGS__)
#define LOG_ERROR_ONLY(...)  SPDLOG_LOGGER_ERROR(Logger::errorLogger(), __VA_ARGS__)

class Logger
{
public:
    static void init(const std::string &logDir = "logs");
    static std::shared_ptr<spdlog::logger> instance();
    static std::shared_ptr<spdlog::logger> debugLogger();
    static std::shared_ptr<spdlog::logger> errorLogger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    static std::shared_ptr<spdlog::logger> m_globalLogger;
    static std::shared_ptr<spdlog::logger> m_debugLogger;
    static std::shared_ptr<spdlog::logger> m_errorLogger;
    static bool m_initialized;
};

#endif // LOGGER_H