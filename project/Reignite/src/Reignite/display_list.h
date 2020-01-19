#ifndef _DISPLAY_LIST_H_
#define _DISPLAY_LIST_H_ 1

#include <vector>
#include <memory>

#include "core.h"
#include "basic_types.h"

namespace Reignite {

  class REIGNITE_API DisplayList {
   public:

    class Command {

      virtual void execute() = 0;
    };

    DisplayList() {}
    ~DisplayList() {}

    void add(const std::shared_ptr<Command> command) {

      data->commands.push_back(command);
    }

    void reset() {

      data->commands.clear();
    }

    DisplayList clone() const {

      DisplayList copy;
      // TODO: Do copy method on implementation
      return copy;
    }

  private:

    struct Data {

      std::vector<std::shared_ptr<Command> > commands;
    };

    Data* data;
  };
}


#endif // _DISPLAY_LIST_H_
