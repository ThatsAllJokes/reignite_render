#ifndef _RI_RENDER_COMPONENT_H_
#define _RI_RENDER_COMPONENT_H_ 1

#include "../basic_types.h"

#include "base_component.h"


namespace Reignite {

  struct RenderComponents : public BaseComponents {
  
    struct Textures {
      s32 normalMap = -1;
      s32 colorMap = -1;
      s32 roughness = -1;
      s32 metallic = -1;
    };

    void init(u32 maxSize);
    void clear();

    void add();

    void update();

    std::vector<u32> geometry;
    std::vector<u32> material;
    std::vector<Textures> texture;
  };

}

#endif // _RI_RENDER_CONTEXT_H_