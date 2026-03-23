
module;
#include <memory>
export module GeantCore.Core.Commands.CommandManager;

import GeantCore.Utils.Event;
export namespace GeantCore::Core::Commands {
class CommandManager{
public:
  void ApplyCommand() 
  {
    ExperimentAppliedEvent.Invoke();
  }

public: 
 GeantCore::Utils::Event<>& GetExperimentAppliedEvent(){return ExperimentAppliedEvent;}

private:

private:
  GeantCore::Utils::Event<> ExperimentAppliedEvent;
};
} // namespace GeantCore::Core::Commands
