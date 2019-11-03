#include "timer.h"


std::chrono::time_point<std::chrono::steady_clock> Reignite::Timer::s_time_start;
std::chrono::time_point<std::chrono::steady_clock> Reignite::Timer::s_time_end;

void Reignite::Timer::StartTime() {

  s_time_start = std::chrono::high_resolution_clock::now();
}

std::chrono::seconds Reignite::Timer::EndTime() {

  s_time_end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(s_time_end - s_time_start);
}