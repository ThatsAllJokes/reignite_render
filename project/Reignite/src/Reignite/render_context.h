#ifndef _RI_RENDER_CONTEXT_H_
#define _RI_RENDER_CONTEXT_H_ 1

#include <memory>
#include <string>

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
    
    enum GeometryEnum {
       kGeometryEnum_Square,
       kGeometryEnum_Cube,
       kGeometryEnum_Load,
       kGeometryEnum_Terrain
    };

    RenderContext(const std::shared_ptr<State> state);
    ~RenderContext();

    u32 createGeometryResource(GeometryEnum geometry, std::string path = "");
    u32 createMaterialResource();
    u32 createTextureResource(std::string filename);

    void initRenderState();
    void updateRenderState();

    void buildDeferredCommands();
    void buildCommandBuffers();

    void windowResize();

    void drawScene();
    void drawOverlay();

   private:

    void createCommandBuffers();
    void destroyCommandBuffers();


    void setupDepthStencil();
    void setupFramebuffer();

    void updateUniformBuffersScreen();
    void updateUniformBufferDeferredMatrices();
    void updateUniformBufferDeferredLights();

    void loadResources();

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

