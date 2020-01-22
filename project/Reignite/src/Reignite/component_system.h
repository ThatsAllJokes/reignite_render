#ifndef _RI_COMPONENT_SYSTEM_H_
#define _RI_COMPONENT_SYSTEM_H_ 1

#include "core.h"


namespace Reignite {

  struct State;

  class REIGNITE_API ComponentSystem {
   public:

    ComponentSystem(const State* state);
    ~ComponentSystem();

    void update();

   private:

    bool init(const State* state);
    void shutdown();

    State* state;
  };

} // end of Reignite namespace

#endif // _RI_COMPONENT_SYSTEM_H_



