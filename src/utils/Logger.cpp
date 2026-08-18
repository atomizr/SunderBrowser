#include "Logger.h"
#include <QCoreApplication>
#include <QDir>

std::shared_ptr<spdlog::logger> Logger::m_globalLogger = nullptr;
std::shared_ptr<spdlog::logger> Logger::m_debugLogger = nullptr;
std::shared_ptr<spdlog::logger> Logger::m_errorLogger = nullptr;
bool Logger::m_initialized = false;

void Logger::init(const std::string &logDir)
{
    if (m_initialized)
        return;

    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);
    QString fullLogDir = dir.filePath(QString::fromStdString(logDir));
    QDir().mkpath(fullLogDir);

    std::string logPath = fullLogDir.toStdString();

    auto global_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath + "/sunder.log", 1024 * 1024 * 5, 3);
    auto debug_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath + "/debug.log", 1024 * 1024 * 5, 3);
    auto error_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath + "/error.log", 1024 * 1024 * 5, 3);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    m_globalLogger = std::make_shared<spdlog::logger>("global", spdlog::sinks_init_list{global_sink, console_sink});
    m_globalLogger->set_level(spdlog::level::trace);
    m_globalLogger->flush_on(spdlog::level::trace);
    m_globalLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

    m_debugLogger = std::make_shared<spdlog::logger>("debug", debug_sink);
    m_debugLogger->set_level(spdlog::level::debug);
    m_debugLogger->flush_on(spdlog::level::debug);
    m_debugLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

    m_errorLogger = std::make_shared<spdlog::logger>("error", error_sink);
    m_errorLogger->set_level(spdlog::level::err);
    m_errorLogger->flush_on(spdlog::level::err);
    m_errorLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

    spdlog::register_logger(m_globalLogger);
    spdlog::register_logger(m_debugLogger);
    spdlog::register_logger(m_errorLogger);

    m_initialized = true;
    m_globalLogger->info("Logger initialized. Log directory: {}", fullLogDir.toStdString());
}

std::shared_ptr<spdlog::logger> Logger::instance()
{
    if (!m_initialized)
        init();
    return m_globalLogger;
}

std::shared_ptr<spdlog::logger> Logger::debugLogger()
{
    if (!m_initialized)
        init();
    return m_debugLogger;
}

std::shared_ptr<spdlog::logger> Logger::errorLogger()
{
    if (!m_initialized)
        init();
    return m_errorLogger;
}