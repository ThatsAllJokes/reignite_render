#ifndef _RI_COMPONENT_SYSTEM_H_
#define _RI_COMPONENT_SYSTEM_H_ 1

#include <memory>

#include "core.h"
#include "basic_types.h"


namespace Reignite {

  struct State;

  struct Camera;
  struct TransformComponents;
  struct RenderComponents;
  struct LightComponents; 
  
  struct ComponentSystemParams{

    u32 max_entities = 40;
  };

  class REIGNITE_API ComponentSystem {
   public:

    ComponentSystem(const std::shared_ptr<State> state);
    ~ComponentSystem();

    void addEntity();
    void addEntity(s32 parentId);

    Camera* camera();
    TransformComponents* transform();
    RenderComponents* render();
    LightComponents* light();

    void update();

   private:

    void initialize(const std::shared_ptr<State> state);
    void shutdown();

    struct Data;
    Data* data;
  };

} // end of Reignite namespace

#endif // _RI_COMPONENT_SYSTEM_H_



