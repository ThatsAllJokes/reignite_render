#ifndef _RI_WINDOW_
#define _RI_WINDOW_ 1

#include <memory>

#include "core.h"
#include "basic_types.h"


namespace Reignite {

  struct State;

  class REIGNITE_API Window {
   public:
    
    Window(const std::shared_ptr<State> state);
    virtual ~Window();

    void update();

    bool closeWindow();

    const u16 getWidth();
    const u16 getHeight();

    //GLFWwindow* const GetCurrentWindow();

    static Window* Create(const std::shared_ptr<State> state);
   
   private:

    void initialize(const std::shared_ptr<State> state);
    void shutdown();

    std::shared_ptr<State> state;
    
    struct Data;
    Data* data;
  };

} // end of Reignite namespace

#endif // _RI_WINDOW_

