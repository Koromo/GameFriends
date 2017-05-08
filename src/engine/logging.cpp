#include "logging.h"

GF_NAMESPACE_BEGIN

LogManager logManager;

void LogManager::startup(const std::string& path)
{
    logger_ = enforce<LoggingError>(spdlog::basic_logger_mt("GameFriends", path.c_str(), true),
        "Log file creation failed (" + path + ")");

    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("***** %+ *****");
    logger_->info("GameFriends log start.");
    spdlog::set_pattern("[%X] [%l] %v");
}

void LogManager::shutdown()
{
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("***** %+ *****");
    logger_->info("GameFriends log finish.");
    spdlog::drop_all();
}

void LogManager::setLevel(LoggingLevel level)
{
    spdlog::set_level(static_cast<spdlog::level::level_enum>(static_cast<int>(level)));
}

GF_NAMESPACE_END