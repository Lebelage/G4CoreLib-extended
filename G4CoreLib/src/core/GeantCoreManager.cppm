module;

/// Geant4
#include "G4RunManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include <memory>
export module GeantCore.Core.GeantCoreManager;

import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Interfaces.IGeantCoreManager;
import GeantCore.Core.Messengers.BaseExperimentMessenger;
export namespace GeantCore::Core {
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

  };
#pragma endregion

private:
#pragma region Fields
  std::unique_ptr<G4UIExecutive> ui;
  std::unique_ptr<G4RunManager> runManager;
  std::unique_ptr<G4VisExecutive> visManager;
  G4UImanager *uiManager = nullptr;

  std::shared_ptr<GeantCore::Models::Experiment::BaseExperimentConfig> config;
#pragma endregion
};

} // namespace GeantCore::Core