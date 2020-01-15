#ifndef _RI_WINDOW_
#define _RI_WINDOW_ 1

#include <string>

#include "core.h"
#include "basic_types.h"
#include "component_system.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Reignite {

  struct State;

  class REIGNITE_API Window {
   public:
    
    Window(const State* state);
    virtual ~Window();

    void update();

    bool closeWindow();

    const u16 getWidth();
    const u16 getHeight();

    inline GLFWwindow* const GetCurrentWindow() { return window; }

    static Window* Create(const State* state);
   
   private:

    void init(const State* state);
    void shutdown();

    GLFWwindow* window;

    State* state;
  };

} // end of Reignite namespace

#endif // _RI_WINDOW_

