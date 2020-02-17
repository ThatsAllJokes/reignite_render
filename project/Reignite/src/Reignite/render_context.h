#ifndef _RI_RENDER_CONTEXT_H_
#define _RI_RENDER_CONTEXT_H_ 1

#include <memory>

#include "core.h"
#include "basic_types.h"


namespace Reignite {

  struct State;

  class DisplayList;

  struct RenderContextParams {

    u32 max_geometries = 128;
    u32 max_materials = 128;
    u32 max_textures = 128;
    u32 max_framebuffers = 128;
  };

  class REIGNITE_API RenderContext {
   public:

    RenderContext(const std::shared_ptr<State> state);
    ~RenderContext();

    u32 createGeometry();
    u32 createMaterial();
    u32 createTexture();
    u32 createFrameBuffer();

    void setRenderInfo();

    void submitDisplayList();
    void draw();

   private:

    void updateUniformBuffers();

    void initialize(const std::shared_ptr<State> state, const RenderContextParams& params = RenderContextParams());
    void shutdown();

    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    std::shared_ptr<State> state;
    
    struct Data;
    Data* data;
  };

} // end of Reignite namespace

#endif // _RENDER_CONTEXT_H_

