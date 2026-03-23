module;
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include <memory>
export module GeantCore.Core.SourceGenerators.BaseSourceGenerator;
import GeantCore.Core.Interfaces.ISourceGenerator;
import GeantCore.Models.Experiment.ExperimentConfig;
export namespace GeantCore::Core::SourceGenerators {
using namespace GeantCore::Core::Interfaces;
using namespace GeantCore::Models::Experiment;
class BaseSourceGenerator : public ISourceGenerator {
#pragma region Constructor/Destructor
public:
  BaseSourceGenerator(
      GeantCore::Models::Experiment::BaseExperimentConfig &config)
      : config{config} {
    Initialize();
  };

  ~BaseSourceGenerator() override {};

  BaseSourceGenerator(const BaseSourceGenerator &) = delete;
  BaseSourceGenerator &operator=(const BaseSourceGenerator &) = delete;

  BaseSourceGenerator(const BaseSourceGenerator &&) = delete;
  BaseSourceGenerator &operator=(const BaseSourceGenerator &&) = delete;
#pragma endregion

#pragma region Methods
public:
  void Initialize() { gun = std::make_unique<G4ParticleGun>(1); }

  void GeneratePrimaries(G4Event *event) override {
    if (config.sourceType == SourceType::Gun) {
      auto *def = G4ParticleTable::GetParticleTable()->FindParticle(
          config.gun.particle);
      gun->SetParticleDefinition(def);
      gun->SetParticleEnergy(config.gun.energy);
      gun->SetParticlePosition(config.gun.pos);
      gun->SetParticleMomentumDirection(config.gun.dir.unit());
      gun->GeneratePrimaryVertex(event);
    } else if (config.sourceType == SourceType::Decay) {
      /// TODO
      return;
    }
  }
#pragma endregion

#pragma region Fields
public:
  BaseExperimentConfig &config;
  std::unique_ptr<G4ParticleGun> gun;
#pragma endregion
};
} // namespace GeantCore::Core::SourceGenerators