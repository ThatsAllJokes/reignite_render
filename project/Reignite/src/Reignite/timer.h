#ifndef _RI_TIMER_
#define _RI_TIMER_ 1

#include <chrono>

#include "core.h"

namespace Reignite {

  class REIGNITE_API Timer {
   public:

    static void StartTime();
    static double EndTime();

   private:

    static std::chrono::steady_clock::time_point s_time_start;
    static std::chrono::steady_clock::time_point s_time_end;
  };

} // end of Reignite namespace

#endif // _RI_TIMER_
