#pragma once

#ifdef RI_PLATFORM_WINDOWS
  #ifdef RI_BUILD_DLL
    #define REIGNITE_API __declspec(dllexport)
  #else
    #define REIGNITE_API __declspec(dllimport)
  #endif // RI_BUILD_DLL
#else
  #error Reignite only supports Windows.
#endif // RI_PLATFORM_WINDOWS
