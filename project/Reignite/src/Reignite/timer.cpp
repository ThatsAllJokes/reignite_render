#include "timer.h"


std::chrono::steady_clock::time_point Reignite::Timer::s_time_start;
std::chrono::steady_clock::time_point Reignite::Timer::s_time_end;

void Reignite::Timer::StartTime() {

  s_time_start = std::chrono::high_resolution_clock::now();
}

double Reignite::Timer::EndTime() {

  s_time_end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(s_time_end - s_time_start).count();
}