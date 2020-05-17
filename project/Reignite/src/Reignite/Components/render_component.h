#ifndef _RI_RENDER_COMPONENT_H_
#define _RI_RENDER_COMPONENT_H_ 1

#include "../basic_types.h"

#include "base_component.h"


namespace Reignite {

  struct RenderComponents : public BaseComponents {

    void init(u32 maxSize);
    void clear();

    void add();

    void update();

    std::vector<u32> geometry;
    std::vector<u32> material;
  };

}

#endif // _RI_RENDER_CONTEXT_H_