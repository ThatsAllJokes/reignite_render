solution "Reignite"
  location "project"
  platforms "x64"
  startproject "Render"

  windowstargetplatformversion "10.0.17763.0"

  configurations {
    "Debug",
    "Release"
  }

project "Reignite"
  location "project/Reignite"
  kind "SharedLib"
  language "C++"

  files {
    "extern/glm/glm/**.hpp",
    "extern/imgui/**.cpp",
    "extern/volk/volk.c",
    "extern/stb/stb_image.h",
    "project/Reignite/src/**.h",
    "project/Reignite/src/**.cpp",
    "project/Reignite/src/Commands/**.h",
    "project/Reignite/src/Commands/**.cpp",
    "project/Reignite/src/Components/**.h",
    "project/Reignite/src/Components/**.cpp",
    "project/Reignite/src/GfxResources/**.h",
    "project/Reignite/src/GfxResources/**.cpp",
    "project/Reignite/src/Vulkan/**.h",
    "project/Reignite/src/Vulkan/**.cpp",
  }

  includedirs {
    "$(VULKAN_SDK)\\Include",
    "extern/glfw/include",
    "extern/glm/glm",
    "extern/imgui",
    "extern/spdlog/include",
    "extern/stb",
    "extern/tinygltf",
    "extern/tinyobjloader",
    "extern/volk",
  }

  links {
    "GLFW",
    "C:\\VulkanSDK\\1.1.108.0\\Lib\\vulkan-1",
  }

  configuration "Debug"
  
    targetdir ("bin/Debug/x64/Reignite")
    objdir ("bin-int/Debug/x64/Reignite")

    defines {
      "WIN32_LEAN_AND_MEAN",
      "NOMINMAX",
      "RI_PLATFORM_WINDOWS",
      "RI_BUILD_DLL",
      "RI_DEBUG",
      "_WIN32",
      "_GLFW_WIN32",
      "GLFW_EXPOSE_NATIVE_WIN32",
      "VK_USE_PLATFORM_WIN32_KHR",
      "_CRT_SECURE_NO_WARNINGS"
    }

    flags {
      "Symbols"
    }

    dll_dest = '"$(SolutionDir)..\\bin\\Debug\\x64\\Reignite\\*.dll"'
    dll_targ = '"$(SolutionDir)..\\bin\\Debug\\x64\\Render\\*.dll"'
    postbuildcommands {
      'copy /Y ' .. dll_dest .. ' ' .. dll_targ .. ''
    }
  
  configuration { }
  
  configuration "Release" 

    targetdir ("bin/Release/x64/Reignite")
    objdir ("bin-int/Release/x64/Reignite")

    defines {
      "RI_PLATFORM_WINDOWS",
      "RI_BUILD_DLL",
      "RI_RELEASE"
    }

  configuration { }

  custombuildtask {
    { "project/data/shaders/*.glsl", "project/data/shaders/%(Filename).spv", 
    { "%(FullPath)" }, { "$(VULKAN_SDK)\\Bin\\glslangValidator %(FullPath) -V -o ../data/shaders/%(Filename).spv" } },
  }

project "Render"
  location "project/Render"
  kind "ConsoleApp"
  language "C++"

  files {
    "project/Render/src/**.h",
    "project/Render/src/**.cpp",
  }

  includedirs {
    "extern/glm/glm",
    "extern/spdlog/include",
    "extern/glfw/include",
    "project/Reignite/src",
  }

  links {
    "Reignite"
  }

  configuration "Debug" 
  
    targetdir ("bin/Debug/x64/Render")
    objdir ("bin-int/Debug/x64/Render")

    defines {
      "RI_PLATFORM_WINDOWS",
      "RI_DEBUG"
    }

    flags {
      "Symbols"
    }

  configuration { }

  configuration "configurations:Release" 
  
    targetdir ("bin/Release/x64/Render")
    objdir ("bin-int/Release/x64/Render")

    defines {
      "RI_PLATFORM_WINDOWS",
      "RI_RELEASE"
    }

  configuration { }

-- //////////////////////////////////////////////////////////////////////////////
-- // Side Projects /////////////////////////////////////////////////////////////
-- //////////////////////////////////////////////////////////////////////////////

project "GLFW"
  location "project/GLFW"
  kind "StaticLib"
  language "C"

  files {
    "extern/glfw/include/GLFW/glfw3.h",
    "extern/glfw/include/GLFW/glfw3native.h",
    "extern/glfw/src/glfw_config.h",
    "extern/glfw/src/context.c",
    "extern/glfw/src/init.c",
    "extern/glfw/src/input.c",
    "extern/glfw/src/monitor.c",
    "extern/glfw/src/vulkan.c",
    "extern/glfw/src/window.c"
  } 

  files {
    "extern/glfw/src/win32_init.c",
    "extern/glfw/src/win32_joystick.c",
    "extern/glfw/src/win32_monitor.c",
    "extern/glfw/src/win32_time.c",
    "extern/glfw/src/win32_thread.c",
    "extern/glfw/src/win32_window.c",
    "extern/glfw/src/wgl_context.c",
    "extern/glfw/src/egl_context.c",
    "extern/glfw/src/osmesa_context.c"
  }

  configuration "Debug"
    
    targetdir ("bin/Debug/x64/GLFW")
    objdir ("bin-int/Debug/x64/GLFW")

    defines  { 
      "RI_DEBUG",
      "RI_PLATFORM_WINDOWS",
      "_GLFW_WIN32",
      "_CRT_SECURE_NO_WARNINGS"
    }

  configuration { }

  configuration "Release"
    
    targetdir ("bin/Release/x64/GLFW")
    objdir ("bin-int/Release/x64/GLFW")

    defines  { 
      "RI_RELEASE",
      "RI_PLATFORM_WINDOWS",
      "_GLFW_WIN32",
      "_CRT_SECURE_NO_WARNINGS"
    }

  configuration { }
