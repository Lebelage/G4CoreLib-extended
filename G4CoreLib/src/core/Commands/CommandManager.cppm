
module;
#include <memory>
export module GeantCore.Core.Commands.CommandManager;
import GeantCore.Core.EventManager;
export namespace GeantCore::Core::Commands {
using namespace GeantCore::Core;
class CommandManager {
public:
  void ApplyCommand() { EventManager::GetGeometryUpdatedEvent().Invoke(); }
};
} // namespace GeantCore::Core::Commands
