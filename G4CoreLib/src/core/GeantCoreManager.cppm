module;
#include <memory>
export module GeantCore.Core.GeantCoreManager;
import GeantCore.Externals;
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
import GeantCore.Core.SourceGenertors.SourceGeneratorManager;
export namespace GeantCore::Core {
    using namespace GeantCore::Core::Interfaces;
    using namespace GeantCore::Core::Detectors;
    using namespace GeantCore::Core::Messengers;
    using namespace GeantCore::Core::SourceGenerators;

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
                [this](std::shared_ptr<BaseExperimentConfig> config) { this->OnExperimentUpdated(config); });
        }

        ~GeantCoreManager() {
        }
#pragma endregion

    public:
#pragma region Methods
        void Initialize(int argc, char **argv,
                        std::shared_ptr<BaseExperimentConfig> config = nullptr) {
            this->config = std::make_shared<BaseExperimentConfig>();
            InitializeRunManager(argc, argv);
        }

        void InitializeUI(int argc, char **argv) {
            uiManager = GeantModules::G4UImanager::GetUIpointer();

            const auto isInteractive = (argc == 1);
            if (!isInteractive) {

                std::string fullCommand;
                // Batch mode: запускаем пе реданный макрос
                GeantModules::G4String command = "/control/execute ";
                fullCommand += command;
                fullCommand += argv[1];
                uiManager->ApplyCommand(GeantModules::G4String(fullCommand));
                return;
            }

            visManager = std::make_unique<GeantModules::G4VisExecutive>();
            visManager->Initialize();

            uiManager->ApplyCommand("/control/macroPath AppConfigs");
            uiManager->ApplyCommand("/control/execute init.mac");

            ui->SessionStart();
        }

    private:
        void InitializeRunManager(int argc, char **argv) {
            const bool isInteractive = (argc == 1);

            if (isInteractive)
                ui = std::make_unique<GeantModules::G4UIExecutive>(argc, argv);

            runManager = std::make_unique<GeantModules::G4RunManager>();
            expMessenger = std::make_unique<BaseExperimentMessenger>();
            detManager = std::make_unique<DetectorManager>();

            //runManager->SetNumberOfThreads(8);

            detManager->SetDetector(std::make_unique<StartDetector>());

            auto *physics = InitializePhysics();
            runManager->SetUserInitialization(physics);

            sourceManager = std::make_unique<SourceGeneratorManager>();
            sourceManager->SetSourceGenerator(std::make_unique<BaseSourceGenerator>(nullptr));

            runManager->SetUserInitialization(detManager->GetCurrentDetectorPointer());
            runManager->SetUserAction(sourceManager->GetCurrentSourceGeneratorPointer());

            runManager->SetUserAction(new GeantCore::Core::Actions::BaseRunAction());
            //runManager->SetUserAction(new GeantCore::Core::Actions::BaseSteppingAction());

            runManager->Initialize();
        };

    private:
        GeantModules::FTFP_BERT *InitializePhysics() {
            auto *physics = new GeantModules::FTFP_BERT();
            physics->ReplacePhysics(new GeantModules::G4EmStandardPhysics_option4());
            physics->RegisterPhysics(new GeantModules::G4StepLimiterPhysics());
            physics->RegisterPhysics(new GeantModules::G4DecayPhysics());
            physics->RegisterPhysics(new GeantModules::G4RadioactiveDecayPhysics());

            return physics;
        }

#pragma endregion

    private:
#pragma region Fields
        std::unique_ptr<GeantModules::G4UIExecutive> ui;
        std::unique_ptr<GeantModules::G4RunManager> runManager;
        std::unique_ptr<GeantModules::G4VisExecutive> visManager;
        GeantModules::G4UImanager *uiManager = nullptr;

        std::unique_ptr<IExperimentMessenger> expMessenger;
        std::unique_ptr<DetectorManager> detManager;
        std::unique_ptr<SourceGeneratorManager> sourceManager;
        std::shared_ptr<BaseExperimentConfig> config;

#pragma endregion

    public:
#pragma region Properties

#pragma endregion

#pragma region Handlers

    public:
        void OnExperimentUpdated(std::shared_ptr<BaseExperimentConfig> config) {
            uiManager->ApplyCommand("/vis/disable");

            detManager->SetDetector(std::make_unique<BaseDetectorConstruction>(config));

            detManager->ApplyConfigChanges();
            detManager->GetCurrentDetectorPointer()->ConstructSDandField();

            sourceManager->SetSourceGenerator(std::make_unique<BaseSourceGenerator>(config));

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
