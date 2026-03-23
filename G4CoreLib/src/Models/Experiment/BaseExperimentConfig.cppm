module;
/// Geant4
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

/// std
#include <unordered_map>
#include <vector>
export module GeantCore.Models.Experiment.ExperimentConfig;

export namespace GeantCore::Models::Experiment {

enum class ExpType { None, Stack };

enum class SourceType { Gun, Decay };

struct LayerSpec {
  G4String material;
  double thickness; // mm (G4 units)
};

struct ElementMassSpec {
  G4String element;
  double massFraction; // should sum to 1
};

struct MaterialBuildSpec {
  double density = 0.0; // g/cm3 in G4 units after parsing
  bool finalized = false;

  // choose one mode:
  bool useAtoms = true; // true => atoms list, false => mass list
  std::vector<ElementMassSpec> mass;
};

struct GunSource {
  G4String particle = "e-";
  double energy = 100 * keV;
  G4ThreeVector pos{0, 0, 0};
  G4ThreeVector dir{0, 0, 1};
};
class BaseExperimentConfig {
public:
  virtual ~BaseExperimentConfig() = default;

public:
  // world
  G4String worldMaterial = "G4_Galactic";
  double worldSize = 1.0; // mm

  // type
  ExpType type = ExpType::None;

  // stack
  double stackX = 100.0; // mm
  double stackY = 100.0; // mm
  G4ThreeVector stackPos{0, 0, 0};
  std::vector<LayerSpec> layers;

  // source
  SourceType sourceType = SourceType::Gun;
  GunSource gun;

  // materials defined from macro
  std::unordered_map<std::string, MaterialBuildSpec> matBuild;

  // dirty flags
  bool geomDirty = false;
  bool physDirty = false;
  bool beamDirty = false;
};
} // namespace GeantCore::Models::Experiment