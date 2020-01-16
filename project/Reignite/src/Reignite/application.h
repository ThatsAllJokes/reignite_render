#ifndef _RI_APPLICATION_H_
#define _RI_APPLICATION_H_ 1

#include <memory>
#include <vector>

#include "core.h"
#include "window.h"
#include "component_system.h"

namespace Reignite {

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
    //TODO: scheduling system
    std::unique_ptr<ComponentSystem> component_system;
  };

  // To be defined in client
  Application* CreateApplication();
}

#endif // _RI_APPLICATION_H_

