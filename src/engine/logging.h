#ifndef GAMEFRIENDS_LOGGING_H
#define GAMEFRIENDS_LOGGING_H

#include "foundation/exception.h"
#include "foundation/prerequest.h"
#include <spdlog/spdlog.h>

GF_NAMESPACE_BEGIN

class LoggingError : public Error
{
public:
    explicit LoggingError(const std::string& msg)
        : Error(msg) {}
};

enum LoggingLevel
{
    LL_trace = 0,
    LL_debug = 1,
    LL_info = 2,
    LL_warn = 3,
    LL_error = 4,
    LL_critical = 5,
    LL_off = 6,
};

class LogManager
{
private:
    std::shared_ptr<spdlog::logger> logger_;

public:
    void startup(const std::string& path) noexcept(false);
    void shutdown();

    void setLevel(LoggingLevel level);

    template <class Arg1, class... Args>
    void trace(const char* fmt, const Arg1& arg1, const Args&... args)
    {
        logger_->trace(fmt, arg1, args...);
    }

    template <class Arg1, class... Args>
    void debug(const char* fmt, const Arg1& arg1, const Args&... args)
    {
        logger_->debug(fmt, arg1, args...);
    }

    template <class Arg1, class... Args>
    void info(const char* fmt, const Arg1& arg1, const Args&... args)
    {
        logger_->info(fmt, arg1, args...);
    }

    template <class Arg1, class... Args>
    void warn(const char* fmt, const Arg1& arg1, const Args&... args)
    {
        logger_->warn(fmt, arg1, args...);
    }

    template <class Arg1, class... Args>
    void error(const char* fmt, const Arg1& arg1, const Args&... args)
    {
        logger_->error(fmt, arg1, args...);
    }

    template <class Arg1, class... Args>
    void critical(const char* fmt, const Arg1& arg1, const Args&... args)
    {
        logger_->critical(fmt, arg1, args...);
    }

    template <class T>
    void trace(const T& msg)
    {
        logger_->trace(msg);
    }

    template <class T>
    void debug(const T& msg)
    {
        logger_->debug(msg);
    }

    template <class T>
    void info(const T& msg)
    {
        logger_->info(msg);
    }

    template <class T>
    void warn(const T& msg)
    {
        logger_->warn(msg);
    }

    template <class T>
    void error(const T& msg)
    {
        logger_->error(msg);
    }

    template <class T>
    void critical(const T& msg)
    {
        logger_->critical(msg);
    }
};

extern LogManager logManager;

GF_NAMESPACE_END

#define GF_LOG_TRACE(...) GF_NAMESPACE::logManager.trace(__VA_ARGS__)
#define GF_LOG_DEBUG(...) GF_NAMESPACE::logManager.debug(__VA_ARGS__)
#define GF_LOG_INFO(...) GF_NAMESPACE::logManager.info(__VA_ARGS__)
#define GF_LOG_WARN(...) GF_NAMESPACE::logManager.warn(__VA_ARGS__)
#define GF_LOG_ERROR(...) GF_NAMESPACE::logManager.error(__VA_ARGS__)
#define GF_LOG_CRITICAL(...) GF_NAMESPACE::logManager.critical(__VA_ARGS__)
#define GF_LOG_LEVEL_SET(level) GF_NAMESPACE::logManager.setLevel(level)

#endif