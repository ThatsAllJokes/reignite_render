#ifndef _RI_WINDOW_
#define _RI_WINDOW_ 1

#include <string>

#include "core.h"
#include "basic_types.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Reignite {

  struct VkData;

  struct WindowParams {

    std::string title;
    u16 width;
    u16 height;

    WindowParams(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  class REIGNITE_API Window {
   public:
    
    Window(const WindowParams& params);
    virtual ~Window();

    void update();

    inline const u16 getWidth() { return params.width; }
    inline const u16 getHeight() { return params.height; }

    static Window* Create(const WindowParams& params = WindowParams());
   
   private:

    void init(const WindowParams& params);
    void shutdown();

    GLFWwindow* window;

    VkData* vk_data;
    WindowParams params;
  };

}

#endif // _RI_WINDOW_

