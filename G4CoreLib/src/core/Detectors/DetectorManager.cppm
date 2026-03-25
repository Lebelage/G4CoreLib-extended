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

export namespace GeantCore::Core::Detectors {

using namespace GeantCore::Models::Experiment;
using namespace GeantCore::Core::Interfaces;
using namespace GeantCore::Core::Concepts;

class DetectorManager {
#pragma region Constructor
public:
  DetectorManager(GeantCore::Models::Experiment::BaseExperimentConfig &config)
      : config{config} {};
  ~DetectorManager() {};

  DetectorManager(const DetectorManager &) = delete;
  DetectorManager &operator=(const DetectorManager &) = delete;

  DetectorManager(const DetectorManager &&) = delete;
  DetectorManager &operator=(const DetectorManager &&) = delete;
#pragma endregion

#pragma region Methods
public:
  template <DetectorConstructionConcept Detector>
  void SetDetector(std::unique_ptr<Detector> detector) {
    currentDetector = std::move(detector);
  }

  G4VUserDetectorConstruction *GetCurrentDetectorPointer() {
    return currentDetector.release();
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
    auto *det = const_cast<IDetectorConstruction *>(
        static_cast<const IDetectorConstruction *>(detBase));

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

#pragma region Fields
public:
  BaseExperimentConfig &config;
  std::unique_ptr<G4VUserDetectorConstruction> currentDetector;
#pragma endregion
};
} // namespace GeantCore::Core::Detectors
