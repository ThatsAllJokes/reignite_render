#ifndef _RI_STATE_
#define _RI_STATE_ 1

#include <memory>

#include "basic_types.h"

#include "window.h"
#include "component_system.h"
#include "render_context.h"


namespace Reignite {

  struct State {

    float frameTimer;

    std::shared_ptr<Window> window;
    Input* input;

    vec2f mousePos;
    struct {
      bool left = false;
      bool right = false;
      bool middle = false;
    } mouseButtons;

    std::shared_ptr<ComponentSystem> compSystem;
  };

} // end of Reignite namespace


#endif // _RI_STATE_

