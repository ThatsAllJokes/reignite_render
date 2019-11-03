#ifndef _RI_LOG_
#define _RI_LOG_ 1

#include <memory>

#include "core.h"
#include "spdlog/spdlog.h"

namespace Reignite {

  class REIGNITE_API Log {
   public:

    static void Init();

    inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }

    private:

     static std::shared_ptr<spdlog::logger> s_logger;
  };
}

#define RI_INFO(...)  ::Reignite::Log::GetLogger()->info(__VA_ARGS__)
#define RI_TRACE(...) ::Reignite::Log::GetLogger()->trace(__VA_ARGS__)
#define RI_WARN(...)  ::Reignite::Log::GetLogger()->warn(__VA_ARGS__)
#define RI_ERROR(...) ::Reignite::Log::GetLogger()->error(__VA_ARGS__)
#define RI_FATAL(...) ::Reignite::Log::GetLogger()->fatal(__VA_ARGS__)

#endif // _RI_LOG_

