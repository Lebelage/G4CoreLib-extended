module;

/// Geant4
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4RunManager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include <memory>
export module GeantCore.Core.GeantCoreManager;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Interfaces.IGeantCoreManager;
import GeantCore.Core.Interfaces.IDetectorConstruction;
import GeantCore.Core.Interfaces.IExperimentMessenger;
import GeantCore.Core.Messengers.BaseExperimentMessenger;
import GeantCore.Core.Detectors.DetectorManager;
import GeantCore.Core.SourceGenerators.BaseSourceGenerator;
import GeantCore.Core.Interfaces.IRunAction;
import GeantCore.Core.Actions.BaseRunAction;
import GeantCore.Core.Actions.BaseSteppingAction;

export namespace GeantCore::Core {
using namespace GeantCore::Core::Interfaces;
using namespace GeantCore::Core::Detectors;
class GeantCoreManager : public Interfaces::IGeantCoreManager {
public:
#pragma region Constructors/Destructor

  explicit GeantCoreManager();
  ~GeantCoreManager() {};

  GeantCoreManager(const GeantCoreManager &) = delete;
  GeantCoreManager &operator=(const GeantCoreManager &) = delete;

  GeantCoreManager(const GeantCoreManager &&) = delete;
  GeantCoreManager &operator=(const GeantCoreManager &&) = delete;
#pragma endregion

public:
#pragma region Methods

  void Initialize(
      int argc, char **argv,
      std::shared_ptr<GeantCore::Models::Experiment::BaseExperimentConfig>
          config = nullptr) override {
    InitializeRunManager(argc, argv);
  }

private:
  void InitializeRunManager(int argc, char **argv) {
    const bool isInteractive = (argc == 1);

    if (isInteractive)
      ui = std::make_unique<G4UIExecutive>(argc, argv);

    runManager = std::make_unique<G4RunManager>();
    detManager = std::make_unique<DetectorManager>(*config);
    detManager->ApplyConfigChanges();
  };

private:
  FTFP_BERT *InitializePhysics() {
    auto *physics = new FTFP_BERT();
    physics->ReplacePhysics(new G4EmStandardPhysics_option4());
    physics->RegisterPhysics(new G4StepLimiterPhysics());

    return physics;
  }
#pragma endregion

private:
#pragma region Fields
  std::unique_ptr<G4UIExecutive> ui;
  std::unique_ptr<G4RunManager> runManager;
  std::unique_ptr<G4VisExecutive> visManager;
  G4UImanager *uiManager = nullptr;

  std::shared_ptr<GeantCore::Models::Experiment::BaseExperimentConfig> config;
  std::unique_ptr<IExperimentMessenger> expMessenger;
  std::unique_ptr<DetectorManager> detManager;
#pragma endregion
};

} // namespace GeantCore::Core