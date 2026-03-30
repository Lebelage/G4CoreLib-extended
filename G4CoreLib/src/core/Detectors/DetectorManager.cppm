module;

#include "G4GeometryManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4SolidStore.hh"
#include "G4StateManager.hh"
#include "G4Threading.hh"
#include <memory>

export module GeantCore.Core.Detectors.DetectorManager;

import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Interfaces.IDetectorConstruction;
import GeantCore.Core.Concepts.DetectorConstructionConcept;
import GeantCore.Core.Detectors.DetectorWrapper;
import GeantCore.Core.EventManager;

export namespace GeantCore::Core::Detectors {
    using namespace GeantCore::Models::Experiment;
    using namespace GeantCore::Core::Interfaces;
    using namespace GeantCore::Core::Concepts;

    class DetectorManager
    {
#pragma region Constructor

    public:
        DetectorManager() {
            detWrapper = std::make_unique<DetectorWrapper>();

            // GeantCore::Core::EventManager::GetSteppingAction().Add(
            //     [this](const G4Step* step) { this->OnSteppingAction(step); }
            // );
            //
            EventManager::GetActionCompleted().Add(
              [this](){this->OnActionCompleted(); }
            );
        };

        ~DetectorManager() {
        };

        DetectorManager(const DetectorManager &) = delete;

        DetectorManager &operator=(const DetectorManager &) = delete;

        DetectorManager(const DetectorManager &&) = delete;

        DetectorManager &operator=(const DetectorManager &&) = delete;
#pragma endregion

#pragma region Methods

    public:
        template<DetectorConstructionConcept Detector>
        void SetDetector(std::unique_ptr<Detector> detector) {
            detWrapper->SetBuilder(std::move(detector));
        }

        G4VUserDetectorConstruction *GetCurrentDetectorPointer() {
            return detWrapper.get();
        }

        void ApplyConfigChanges() {
            if (!G4Threading::IsMasterThread()) {
                G4Exception("DetectorManager", "GeomUpdate-NotMaster", JustWarning,
                            "Geometry update requested from worker thread. Ignore.");
                return;
            }

            auto state = G4StateManager::GetStateManager()->GetCurrentState();
            if (state != G4State_Idle) {
                G4Exception("DetectorManager", "GeomUpdate-RunActive", JustWarning,
                            "Run is active or geometry is closed. Stop run before "
                            "updating geometry.");
                return;
            }

            auto *rm = G4RunManager::GetRunManager();

            auto *detBase = rm->GetUserDetectorConstruction();
            auto *det = const_cast<DetectorWrapper *>(
                static_cast<const DetectorWrapper *>(detBase));

            if (!det) {
                G4Exception("DetectorManager", "GeomUpdate-NoDet", FatalException,
                            "No DetectorConstruction registered.");
                return;
            }

            G4GeometryManager::GetInstance()->OpenGeometry();
            G4PhysicalVolumeStore::GetInstance()->Clean();
            G4LogicalVolumeStore::GetInstance()->Clean();
            G4SolidStore::GetInstance()->Clean();

            G4VPhysicalVolume *newWorld = det->BuildWorld();

            rm->DefineWorldVolume(newWorld, /*topologyIsChanged*/ true);

            rm->GeometryHasBeenModified();
        }
#pragma endregion

#define region Handlers

        void OnSteppingAction(const G4Step *step) {
            if (detWrapper) {
                detWrapper->Analyze(step);
            }
        }

        void OnActionCompleted(const G4Step *step) {
            if (detWrapper) {
                detWrapper->Analyze(step);
            }
        }

#define endregion

#pragma region Fields

    public:
        std::unique_ptr<DetectorWrapper> detWrapper;
#pragma endregion
    };
} // namespace GeantCore::Core::Detectors
