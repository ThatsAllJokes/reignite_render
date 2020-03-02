#ifndef _RI_RENDER_COMPONENT_H_
#define _RI_RENDER_COMPONENT_H_ 1

#include "base_component.h"
#include "../basic_types.h"


namespace Reignite {

  struct RenderComponent : public BaseComponent {
  
    RenderComponent();

    s16 geometry;
    s16 material;
    s16 texture;
  };

  void UpdateRenderComponent(RenderComponent& rc);
}

#endif // _RI_RENDER_CONTEXT_H_