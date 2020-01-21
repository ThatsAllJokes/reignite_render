#ifndef _RENDER_CUBE_DEMO_COMMAND_
#define _RENDER_CUBE_DEMO_COMMAND_ 1

#include "base_command.h"


namespace Reignite {

  class RenderCubeDemoCommand : public BaseCommand {
   public:

    void execute() override;
  };

} // end of Reignite namespace

#endif // _RENDER_CUBE_DEMO_COMMAND_
