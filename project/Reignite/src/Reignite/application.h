#ifndef _RI_APPLICATION_H_
#define _RI_APPLICATION_H_ 1

#include "core.h"

namespace Reignite {

  class REIGNITE_API Application {
   public:

    void Run();

   private:

    void Init();

   protected:

    Application();
    virtual ~Application();
  };

  // To be defined in client
  Application* CreateApplication();
}

#endif // _RI_APPLICATION_H_

