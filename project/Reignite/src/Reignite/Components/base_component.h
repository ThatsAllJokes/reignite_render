#ifndef _RI_BASE_COMPONENT_
#define _RI_BASE_COMPONENT_ 1

#include <vector>

#include "../basic_types.h"


namespace Reignite {

  struct BaseComponents {
  
    // Size of current components
    u32 size;

    // Defines if component is being used by the entity
    std::vector<bool> used;

    // Defines if component is active. Entity is already using it
    std::vector<bool> active;
  };

} // end of Reignite namespace

#endif // _RI_BASE_COMPONENT_

