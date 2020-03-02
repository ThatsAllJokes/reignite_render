#ifndef _RI_BASE_COMPONENT_
#define _RI_BASE_COMPONENT_ 1


namespace Reignite {

  struct BaseComponent {
  
    BaseComponent() : is_used(true), is_active(true) {}

    bool is_used;
    bool is_active;
  };

} // end of Reignite namespace

#endif // _RI_BASE_COMPONENT_

