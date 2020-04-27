#ifndef _RI_WINDOW_
#define _RI_WINDOW_ 1

#include <memory>
#include <vector>
#include <map>

#include "core.h"
#include "basic_types.h"


struct GLFWwindow;

namespace Reignite {

  struct State;

  class REIGNITE_API Window {
   public:
    
    Window(const std::shared_ptr<State> state);
    virtual ~Window();

    void update();
    void updateMouseInput();

    bool closeWindow();

    const u16 width();
    const u16 height();
    const std::string title();

    void* const currentWindow();

    static Window* Create(const std::shared_ptr<State> state);
   
   private:

    void initialize(const std::shared_ptr<State> state);
    void shutdown();

    std::shared_ptr<State> state;
    
    struct Data;
    Data* data;
  };

  class REIGNITE_API Input {
   public:

    Input(std::vector<s32> keysToMonitor);
    ~Input();

    bool isKeyDown(s32 key);

    bool isEnabled() { return data->isEnabled; }
    void setIsEnabled(bool value) { data->isEnabled = value; }

    // Must be called in order before any input instances to work
    static void setupKeyInputs(Window& window);

   private:

    void setIsKeyDown(s32 key, bool isDown);

    static void callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods);

    struct Data {
      std::map<s32, bool> keys;
      bool isEnabled;

    };
    Data* data;

    static std::vector<Input*> s_instances;
  };

} // end of Reignite namespace

#endif // _RI_WINDOW_

