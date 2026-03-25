module;
#include "G4String.hh"
#include "G4Tokenizer.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcommand.hh"
#include "G4UImessenger.hh"
#include <iostream>
#include <memory>
export module GeantCore.Core.Messengers.BaseExperimentMessenger;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Interfaces.IExperimentMessenger;
import GeantCore.Core.Commands.CommandManager;
import GeantCore.Core.Commands.G4CommandBuilder;

export namespace GeantCore::Core::Messengers {
using namespace GeantCore::Models::Experiment;
using namespace GeantCore::Core::Interfaces;
using namespace GeantCore::Core::Commands;

class BaseExperimentMessenger : public IExperimentMessenger {
#pragma region Constructor/Destructor
public:
  BaseExperimentMessenger(
      GeantCore::Models::Experiment::BaseExperimentConfig &config)
      : fCfg{config} {
    commandManager = std::make_unique<CommandManager>();
    G4CommandBuilder builder(this);

    fDir = builder.Directory("/exp/", "Experiment configuration");

    fReset = builder.Command("/exp/reset");
    fType = builder.String("/exp/type");

    fWorldMat = builder.String("/exp/world/material");
    fWorldSize = builder.DoubleUnit("/exp/world/size", "Length");

    fStackXY =
        builder.Params("/exp/stack/xy", Param("x", 'd'), Param("xUnit", 's'),
                       Param("y", 'd'), Param("yUnit", 's'));

    fLayersClear = builder.Command("/exp/layers/clear");

    fLayerAdd = builder.Params("/exp/layer/add", Param("mat", 's'),
                               Param("th", 'd'), Param("unit", 's'));

    fMatCreate = builder.Params("/exp/material/create", Param("name", 's'),
                                Param("density", 'd'), Param("densUnit", 's'));

    fMatAddMass =
        builder.Params("/exp/material/addMassFraction", Param("mat", 's'),
                       Param("el", 's'), Param("fraction", 'd'));

    fMatFinalize = builder.Params("/exp/material/finalize", Param("mat", 's'));

    fSourceType = builder.Candidates("/exp/source/type", "gun decay");

    fGunParticle = builder.String("/exp/source/gun/particle");

    fGunEnergy = builder.DoubleUnit("/exp/source/gun/energy", "Energy");

    fGunPos = builder.Vec3Unit("/exp/source/gun/pos", "Length");

    fGunDir = builder.Params("/exp/source/gun/dir", Param("dx", 'd'),
                             Param("dy", 'd'), Param("dz", 'd'));

    applyCommand = builder.Command("/exp/apply");
  };
  ~BaseExperimentMessenger() override { Release(); };

  BaseExperimentMessenger(const BaseExperimentMessenger &) = delete;
  BaseExperimentMessenger &operator=(const BaseExperimentMessenger &) = delete;

  BaseExperimentMessenger(const BaseExperimentMessenger &&) = delete;
  BaseExperimentMessenger &operator=(const BaseExperimentMessenger &&) = delete;
#pragma endregion

#pragma region Methods
private:
  void SetNewValue(G4UIcommand *cmd, G4String value) override {
    std::cout << "ya vizvan" << std::endl;
    if (cmd == applyCommand) {
      commandManager->ApplyCommand();
    }
    // --------------------------------------------------------
    // RESET
    if (cmd == fReset) {
      fCfg = BaseExperimentConfig{};
      return;
    }

    // --------------------------------------------------------
    // TYPE
    if (cmd == fType) {
      fCfg.type = ParseType(value);
      return;
    }

    // --------------------------------------------------------
    // WORLD
    if (cmd == fWorldMat) {
      fCfg.worldMaterial = value;
      return;
    }

    if (cmd == fWorldSize) {
      fCfg.worldSize = fWorldSize->GetNewDoubleValue(value);
      return;
    }

    // --------------------------------------------------------
    // STACK SIZE
    if (cmd == fStackXY) {
      G4Tokenizer tok(value);
      auto xs = tok();
      auto xu = tok();
      auto ys = tok();
      auto yu = tok();

      fCfg.stackX = std::stod(xs) * G4UIcommand::ValueOf(xu);
      fCfg.stackY = std::stod(ys) * G4UIcommand::ValueOf(yu);

      return;
    }

    // --------------------------------------------------------
    // LAYERS
    if (cmd == fLayersClear) {
      fCfg.layers.clear();
      return;
    }

    if (cmd == fLayerAdd) {
      G4Tokenizer tok(value);
      auto mat = tok();
      auto th = tok();
      auto unit = tok();

      fCfg.layers.push_back({mat, std::stod(th) * G4UIcommand::ValueOf(unit)});

      return;
    }

    // --------------------------------------------------------
    // MATERIALS
    if (cmd == fMatCreate) {
      G4Tokenizer tok(value);
      auto name = tok();
      auto dens = tok();
      auto unit = tok();

      MaterialBuildSpec spec;
      spec.density = std::stod(dens) * G4UIcommand::ValueOf(unit);
      spec.finalized = false;
      spec.useAtoms = true;

      fCfg.matBuild[name] = spec;

      return;
    }

    if (cmd == fMatAddMass) {
      G4Tokenizer tok(value);
      auto mat = tok();
      auto el = tok();
      auto fr = tok();

      fCfg.matBuild[mat].useAtoms = false;
      fCfg.matBuild[mat].mass.push_back({el, std::stod(fr)});

      return;
    }

    if (cmd == fMatFinalize) {
      G4Tokenizer tok(value);
      auto mat = tok();
      fCfg.matBuild[mat].finalized = true;

      return;
    }

    // --------------------------------------------------------
    // SOURCE (Physics)
    if (cmd == fSourceType) {
      fCfg.sourceType = (value == "gun")
                            ? GeantCore::Models::Experiment::SourceType::Gun
                            : GeantCore::Models::Experiment::SourceType::Decay;

      return;
    }

    if (cmd == fGunParticle) {
      fCfg.gun.particle = value;
      return;
    }

    if (cmd == fGunEnergy) {
      fCfg.gun.energy = fGunEnergy->GetNewDoubleValue(value);
      return;
    }

    if (cmd == fGunPos) {
      fCfg.gun.pos = fGunPos->GetNew3VectorValue(value);
      return;
    }

    if (cmd == fGunDir) {
      G4Tokenizer tok(value);
      auto dx = tok();
      auto dy = tok();
      auto dz = tok();

      fCfg.gun.dir =
          G4ThreeVector(std::stod(dx), std::stod(dy), std::stod(dz)).unit();

      return;
    };
  }
  void Release() override {
    delete fMatFinalize;
    delete fMatAddMass;
    delete fMatCreate;
    delete fLayerAdd;
    delete fLayersClear;
    delete fStackXY;
    delete fWorldSize;
    delete fWorldMat;
    delete fType;
    delete fReset;
    delete fDir;
    delete applyCommand;
  }

private:
  static ExpType ParseType(const G4String &s) {
    if (s == "stack")
      return ExpType::Stack;
    return ExpType::None;
  }
#pragma endregion

#pragma region Fields
private:
  GeantCore::Models::Experiment::BaseExperimentConfig &fCfg;

  G4UIdirectory *fDir = nullptr;

  // common
  G4UIcommand *fReset = nullptr;
  G4UIcmdWithAString *fType = nullptr;

  // world
  G4UIcmdWithAString *fWorldMat = nullptr;
  G4UIcmdWithADoubleAndUnit *fWorldSize = nullptr;

  // stack
  G4UIcommand *fStackXY = nullptr;
  G4UIcommand *fLayersClear = nullptr;
  G4UIcommand *fLayerAdd = nullptr;

  // materials
  G4UIcommand *fMatCreate = nullptr;
  G4UIcommand *fMatAddMass = nullptr;
  G4UIcommand *fMatFinalize = nullptr;

  // Source
  G4UIcmdWithAString *fSourceType = nullptr;

  // Gun
  G4UIcmdWithAString *fGunParticle = nullptr;
  G4UIcmdWithADoubleAndUnit *fGunEnergy = nullptr;
  G4UIcmdWith3VectorAndUnit *fGunPos = nullptr;
  G4UIcommand *fGunDir = nullptr;

  // Apply flag
  G4UIcommand *applyCommand;

  std::unique_ptr<CommandManager> commandManager;
#pragma endregion
};
} // namespace GeantCore::Core::Messengers