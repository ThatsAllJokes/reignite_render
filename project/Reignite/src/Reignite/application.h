#ifndef _RI_APPLICATION_H_
#define _RI_APPLICATION_H_ 1

#include <memory>
#include <vector>

#include "core.h"
#include "component_system.h"
#include "render_context.h"

namespace Reignite {

  class Window;

  class REIGNITE_API Application {
   public:

    Application();
    virtual ~Application();

    void Run();

   private:

     void initialize();
     void shutdown();

   protected:

    State* state; //TODO: this could be a shared_ptr as its info will be passed and shared between classes

    struct GFXData;
    GFXData* data;

    bool is_running;
    std::unique_ptr<Window> window;
    //TODO: scheduling system. This will be added as time allows to implementation
    std::unique_ptr<ComponentSystem> component_system;
    std::unique_ptr<RenderContext> render_context;
  };

  // To be defined in client
  Application* CreateApplication();
}

#endif // _RI_APPLICATION_H_

