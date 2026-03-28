
module;
#include <memory>
export module GeantCore.Core.Commands.CommandManager;
import GeantCore.Core.EventManager;
import GeantCore.Models.Experiment.ExperimentConfig;
export namespace GeantCore::Core::Commands {
using namespace GeantCore::Core;
  using namespace GeantCore::Models::Experiment;
class CommandManager {
public:
  void ApplyCommand(std::unique_ptr<BaseExperimentConfig> config) { EventManager::GetGeometryUpdatedEvent().Invoke(); }
};
} // namespace GeantCore::Core::Commands
