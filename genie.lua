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
    "project/Reignite/src/**.h",
    "project/Reignite/src/**.cpp",
  }

  includedirs {
    "extern/spdlog/include"
  }

  configuration "Debug"
  
    targetdir ("bin/Debug/x64/Reignite")
    objdir ("bin-int/Debug/x64/Reignite")

    defines {
      "RI_PLATFORM_WINDOWS",
      "RI_BUILD_DLL",
      "RI_DEBUG"
    }

    --postbuildcommands {
      --"copy bin/Debug/x64/Reignite/Reignite.dll bin/Debug/x64/Render/Reignite.dll"
    --}
  
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

  project "Render"
    location "project/Render"
    kind "ConsoleApp"
    language "C++"

    files {
      "project/Render/src/**.h",
      "project/Render/src/**.cpp",
    }

    includedirs {
      "extern/spdlog/include",
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

    configuration { }
    

    configuration "configurations:Release" 
    
      targetdir ("bin/Release/x64/Render")
      objdir ("bin-int/Release/x64/Render")

      defines {
        "RI_PLATFORM_WINDOWS",
        "RI_RELEASE"
      }

    configuration { }
