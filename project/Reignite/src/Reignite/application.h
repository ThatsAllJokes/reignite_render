#ifndef _RI_APPLICATION_H_
#define _RI_APPLICATION_H_ 1

#include <memory>
#include <vector>

#include "core.h"


namespace Reignite {

  struct State;

  class Window;
  class ComponentSystem;
  class RenderContext;

  class REIGNITE_API Application {
   public:

    Application();
    virtual ~Application();

    void Run();

   private:

     void initialize();
     void shutdown();

   protected:

    std::shared_ptr<State> state;

    bool is_running;
    std::unique_ptr<Window> window;
    std::unique_ptr<ComponentSystem> component_system;
    std::unique_ptr<RenderContext> render_context;
  };

  // To be defined in client
  Application* CreateApplication();
}

#endif // _RI_APPLICATION_H_

