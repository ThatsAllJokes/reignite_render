#ifndef _RI_STATE_
#define _RI_STATE_ 1

#include <memory>
#include <chrono>

#include "basic_types.h"

#include "window.h"
#include "component_system.h"
#include "render_context.h"


namespace Reignite {

  struct State {

    u32 frameCounter = 0;
    u32 lastFrame = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;

    float deltaTime = 0.0f;

    std::shared_ptr<Window> window;
    Input* input;

    std::shared_ptr<ComponentSystem> compSystem;
  };

} // end of Reignite namespace


#endif // _RI_STATE_

