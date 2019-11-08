#ifndef _RI_APPLICATION_H_
#define _RI_APPLICATION_H_ 1

#include <memory>

#include "core.h"
#include "window.h"

namespace Reignite {

  class REIGNITE_API Application {
   public:

    Application();
    virtual ~Application();

    void Run();

   protected:

    bool is_running;
    std::unique_ptr<Window> window;
  };

  // To be defined in client
  Application* CreateApplication();
}

#endif // _RI_APPLICATION_H_

