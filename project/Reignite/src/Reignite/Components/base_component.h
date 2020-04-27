#ifndef _RI_BASE_COMPONENT_
#define _RI_BASE_COMPONENT_ 1

#include <vector>


namespace Reignite {

  struct BaseComponents {
  
    u32 size;
    std::vector<bool> used;
    std::vector<bool> active;
  };

} // end of Reignite namespace

#endif // _RI_BASE_COMPONENT_

