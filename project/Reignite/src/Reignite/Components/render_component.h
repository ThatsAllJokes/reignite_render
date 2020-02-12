#ifndef _RI_RENDER_CONTEXT_H_
#define _RI_RENDER_CONTEXT_H_ 1

#include "../basic_types.h"


namespace Reignite {

  struct RenderComponent {
    s16 geometry;
    s16 material;
    s16 texture;
  };
}

typedef Reignite::RenderComponent RenderComp;

void InitRenderComponent(RenderComp& tf);

void UpdateRenderComponent(RenderComp& tf);

#endif // _RI_RENDER_CONTEXT_H_