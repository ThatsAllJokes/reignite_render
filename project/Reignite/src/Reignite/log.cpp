#include "log.h"

//#include "spdlog/sinks/stdout_color_sinks.h"


std::shared_ptr<spdlog::logger> Reignite::Log::s_logger;

void Reignite::Log::Init() {

  spdlog::set_pattern("%^[%T] %n: %v%$");
  s_logger = spdlog::stderr_color_mt("REIGNITE");
  s_logger->set_level(spdlog::level::trace);
}