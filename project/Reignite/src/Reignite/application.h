#ifndef _APPLICATION_H_
#define _APPLICATION_H_ 1

#include "core.h"

namespace Reignite {

  class REIGNITE_API Application {
   public:

    void Run();

   protected:

    Application();
    virtual ~Application();
  };

  // To be defined in client
  Application* CreateApplication();
}

#endif // _APPLICATION_H_

