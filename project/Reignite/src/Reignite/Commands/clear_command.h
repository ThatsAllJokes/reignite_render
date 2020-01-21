#ifndef _CLEAR_COMMAND_
#define _CLEAR_COMMAND_ 1

#include "base_command.h"


namespace Reignite {

  struct ClearCommandInfo {

    float r, g, b, a;
  };

  class ClearCommand : public BaseCommand {
   public:

    void init(const ClearCommandInfo& commandInfo);

    void execute() override;

   private:

    ClearCommandInfo commandInfo;
  };

} // end of Reignite namespace

#endif // _CLEAR_COMMAND_

