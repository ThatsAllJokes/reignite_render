#ifndef _RI_RENDER_COMPONENT_H_
#define _RI_RENDER_COMPONENT_H_ 1

#include "../basic_types.h"

#include "base_component.h"


namespace Reignite {

  struct RenderComponents : public BaseComponents {
  
    void UpdateRenderComponents();

    std::vector<s16> geometry;
    std::vector<s16> material;
    std::vector<s16> texture;
  };

}

#endif // _RI_RENDER_CONTEXT_H_