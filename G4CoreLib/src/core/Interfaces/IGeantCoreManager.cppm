module;
#include <memory>

export module GeantCore.Core.Interfaces.IGeantCoreManager;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Interfaces.IDetectorConstruction;
export namespace GeantCore::Core::Interfaces {
class IGeantCoreManager {
public:
  IGeantCoreManager() = default;
  virtual ~IGeantCoreManager() = default;

  IGeantCoreManager(const IGeantCoreManager &) = delete;
  IGeantCoreManager &operator=(const IGeantCoreManager &) = delete;

  IGeantCoreManager(const IGeantCoreManager &&) = delete;
  IGeantCoreManager &operator=(const IGeantCoreManager &&) = delete;

public:
  virtual void Initialize(
      int argc, char **argv,
      std::shared_ptr<GeantCore::Models::Experiment::BaseExperimentConfig>
          config = nullptr) = 0;
};
} // namespace GeantCore::Core::Interfaces