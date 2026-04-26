module;
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
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
  BaseSourceGenerator(std::shared_ptr<BaseExperimentConfig> config)
      : config{std::move(config)} {
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

    if (config == nullptr) return;
    if (config->sourceType == SourceType::Gun) {
      auto *def = G4ParticleTable::GetParticleTable()->FindParticle(
          config->gun.particle);
      gun->SetParticleDefinition(def);
      gun->SetParticleEnergy(config->gun.energy);
      gun->SetParticlePosition(config->gun.pos);
      gun->SetParticleMomentumDirection(config->gun.dir.unit());
      gun->GeneratePrimaryVertex(event);
    } else if (config->sourceType == SourceType::Decay) {
      auto iontTable = G4IonTable::GetIonTable();
      auto ni63 = iontTable->GetIon(28, 63, 0.0);

      gun->SetParticleDefinition(ni63);
      gun->SetParticleEnergy(0.0 * keV);

      // 1. Находим суммарную толщину никеля (Source)
      double totalSourceZ = 0.0;
      for (const auto &L : config->layers) {
        if (L.material == "Ni63_Source") {
          totalSourceZ += L.thickness;
        }
      }

      // 2. Генерируем точку распада внутри объема никеля
      // В новой геометрии никель начинается от stackPos.z() и идет вверх
      double zMin = config->stackPos.z();
      double zMax = config->stackPos.z() + totalSourceZ;

      double z = zMin + G4UniformRand() * (zMax - zMin);
      double x = config->stackPos.x() + (G4UniformRand() - 0.5) * config->stackX;
      double y = config->stackPos.y() + (G4UniformRand() - 0.5) * config->stackY;

      gun->SetParticlePosition(G4ThreeVector(x, y, z));
      gun->GeneratePrimaryVertex(event);
      return;
    }
  }
#pragma endregion

#pragma region Fields
public:
  std::shared_ptr<BaseExperimentConfig> config;
  std::unique_ptr<G4ParticleGun> gun;
#pragma endregion
};
} // namespace GeantCore::Core::SourceGenerators