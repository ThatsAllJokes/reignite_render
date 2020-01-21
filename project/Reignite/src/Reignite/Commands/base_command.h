#ifndef _BASE_COMMAND_
#define _BASE_COMMAND_ 1


namespace Reignite {

  class BaseCommand {
   public:

    virtual void execute() = 0;
  };

} // end of Reignite namespace

#endif // _BASE_COMMAND_
