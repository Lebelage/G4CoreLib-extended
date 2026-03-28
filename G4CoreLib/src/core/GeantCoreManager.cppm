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
import GeantCore.Core.Interfaces.IDetectorConstruction;
import GeantCore.Core.Interfaces.IExperimentMessenger;
import GeantCore.Core.Messengers.BaseExperimentMessenger;
import GeantCore.Core.Detectors.DetectorManager;
import GeantCore.Core.SourceGenerators.BaseSourceGenerator;
import GeantCore.Core.Detectors.DetectorConstruction;
import GeantCore.Core.Interfaces.IRunAction;
import GeantCore.Core.Actions.BaseRunAction;
import GeantCore.Core.Actions.BaseSteppingAction;
import GeantCore.Core.EventManager;
import GeantCore.Core.Detectors.StartDetector;
export namespace GeantCore::Core {
using namespace GeantCore::Core::Interfaces;
using namespace GeantCore::Core::Detectors;
using namespace GeantCore::Core::Messengers;
class GeantCoreManager {
public:
#pragma region Singleton

  static GeantCoreManager &GetInstance() {

    static GeantCoreManager instance;
    return instance;
  }

  GeantCoreManager(const GeantCoreManager &) = delete;
  GeantCoreManager &operator=(const GeantCoreManager &) = delete;

  GeantCoreManager(const GeantCoreManager &&) = delete;
  GeantCoreManager &operator=(const GeantCoreManager &&) = delete;
#pragma endregion

private:
#pragma region Constructors/Destructor
  GeantCoreManager() {
    EventManager::GetGeometryUpdatedEvent().Add(
        [this](std::shared_ptr<BaseExperimentConfig> config) { this->OnUpdateGeometry(config); });
  }
  ~GeantCoreManager() {}
#pragma endregion

public:
#pragma region Methods
  void Initialize(int argc, char **argv,
                  std::shared_ptr<BaseExperimentConfig> config = nullptr) {

    this->config = std::make_shared<BaseExperimentConfig>();
    InitializeRunManager(argc, argv);
  }
  void InitializeUI(int argc, char **argv) {
    uiManager = G4UImanager::GetUIpointer();

    const auto isInteractive = (argc == 1);
    if (!isInteractive) {
      // Batch mode: запускаем пе реданный макрос
      G4String command = "/control/execute ";
      uiManager->ApplyCommand(command + G4String(argv[1]));
      return;
    }

    visManager = std::make_unique<G4VisExecutive>();
    visManager->Initialize();

    uiManager->ApplyCommand("/control/macroPath AppConfigs");
    uiManager->ApplyCommand("/control/execute init.mac");

    ui->SessionStart();
  }

private:
  void InitializeRunManager(int argc, char **argv) {
    const bool isInteractive = (argc == 1);

    if (isInteractive)
      ui = std::make_unique<G4UIExecutive>(argc, argv);

    runManager = std::make_unique<G4RunManager>();
    expMessenger = std::make_unique<BaseExperimentMessenger>();

    detManager = std::make_unique<DetectorManager>(*config);

    detManager->SetDetector(std::make_unique<StartDetector>());

    auto *physics = InitializePhysics();
    runManager->SetUserInitialization(physics);

    runManager->SetUserInitialization(detManager->GetCurrentDetectorPointer());

    runManager->SetUserAction(
        new GeantCore::Core::SourceGenerators::BaseSourceGenerator(*config));
    runManager->SetUserAction(new GeantCore::Core::Actions::BaseRunAction());

    // 4. ОСТОРОЖНО С STEPPING ACTION!
    // Теперь getUserDetectorConstruction возвращает DetectorWrapper*,
    // поэтому dynamic_cast к StartDetector* вернет nullptr!
    // Если вашему SteppingAction не нужен прям специфично StartDetector,
    // лучше передавать nullptr или переписать его логику.
    runManager->SetUserAction(
        new GeantCore::Core::Actions::BaseSteppingAction(nullptr));

    // 5. Инициализируем ядро Geant4
    runManager->Initialize();
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

  std::unique_ptr<IExperimentMessenger> expMessenger;
  std::unique_ptr<DetectorManager> detManager;
  std::shared_ptr<BaseExperimentConfig> config;

#pragma endregion

public:
#pragma region Properties

#pragma endregion

#pragma region Handlers
public:
  void OnUpdateGeometry(std::shared_ptr<BaseExperimentConfig> config)
  {

    uiManager->ApplyCommand("/vis/disable");

    detManager->SetDetector(std::make_unique<BaseDetectorConstruction>(config));
    detManager->ApplyConfigChanges();

    runManager->BeamOn(0);

    uiManager->ApplyCommand("/vis/scene/clear");
    uiManager->ApplyCommand("/vis/scene/create");
    uiManager->ApplyCommand("/vis/scene/add/volume world");
    uiManager->ApplyCommand("/vis/sceneHandler/attach");

    uiManager->ApplyCommand("/vis/enable");

    uiManager->ApplyCommand("/vis/viewer/refresh");
  }
#pragma endregion
};

} // namespace GeantCore::Core