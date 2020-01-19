#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_ 1

#include "core.h"
#include "basic_types.h"

namespace Reignite {

  struct State;

  class DisplayList;

  struct RenderContextParams {

    u32 max_textures = 128;
    u32 max_framebuffers = 128;
  };

  class REIGNITE_API RenderContext {
   public:

    RenderContext(const State* state);
    ~RenderContext();

    void init(const RenderContextParams& params = RenderContextParams());

    void finish();

    u32 createTexture();
    u32 createFrameBuffer();

    void submitDisplayList(DisplayList* displayList);

   private:

    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    State* state;

    // Render state
    bool render_should_close;
    RenderContextParams params;
  };

} // end of Reignite namespace

#endif // _RENDER_CONTEXT_H_

