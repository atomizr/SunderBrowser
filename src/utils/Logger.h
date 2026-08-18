#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define LOG_TRACE(...)    Logger::instance()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Logger::instance()->debug(__VA_ARGS__)
#define LOG_INFO(...)     Logger::instance()->info(__VA_ARGS__)
#define LOG_WARN(...)     Logger::instance()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    Logger::instance()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::instance()->critical(__VA_ARGS__)

#define LOG_DEBUG_ONLY(...)  Logger::debugLogger()->debug(__VA_ARGS__)
#define LOG_ERROR_ONLY(...)  Logger::errorLogger()->error(__VA_ARGS__)

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