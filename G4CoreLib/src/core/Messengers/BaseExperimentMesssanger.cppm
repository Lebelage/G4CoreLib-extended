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
import GeantCore.Core.Materials.MaterialsConstants;

export namespace GeantCore::Core::Messengers {
using namespace GeantCore::Models::Experiment;
using namespace GeantCore::Core::Interfaces;
using namespace GeantCore::Core::Commands;
  using namespace GeantCore::Core::Materials;

class BaseExperimentMessenger : public IExperimentMessenger {
#pragma region Constructor/Destructor
public:
  BaseExperimentMessenger()
  {
    fCfg = std::make_unique<BaseExperimentConfig>();
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

    fMatCreateX = builder.Params("/exp/material/createx", Param("mat", 's'), Param("x", 'd'));

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
    if (cmd == applyCommand) {
      commandManager->ApplyCommand(std::move(fCfg));
    }
    // --------------------------------------------------------
    // RESET
    if (cmd == fReset) {
      fCfg = std::make_unique<BaseExperimentConfig>();
      return;
    }

    // --------------------------------------------------------
    // TYPE
    if (cmd == fType) {
      GetConfigInstance()->type = ParseType(value);
      return;
    }

    // --------------------------------------------------------
    // WORLD
    if (cmd == fWorldMat) {
      GetConfigInstance()->worldMaterial = value;
      return;
    }

    if (cmd == fWorldSize) {
      GetConfigInstance()->worldSize = fWorldSize->GetNewDoubleValue(value);
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

      GetConfigInstance()->stackX = std::stod(xs) * G4UIcommand::ValueOf(xu);
      GetConfigInstance()->stackY = std::stod(ys) * G4UIcommand::ValueOf(yu);

      return;
    }

    // --------------------------------------------------------
    // LAYERS
    if (cmd == fLayersClear) {
      GetConfigInstance()->layers.clear();
      return;
    }

    if (cmd == fLayerAdd) {
      G4Tokenizer tok(value);
      auto mat = tok();
      auto th = tok();
      auto unit = tok();

      GetConfigInstance()->layers.push_back({mat, std::stod(th) * G4UIcommand::ValueOf(unit)});

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

      GetConfigInstance()->matBuild[name] = spec;

      return;
    }

    if (cmd == fMatCreateX) {
      G4Tokenizer tok(value);
      G4String matName = tok();
      double x = std::stod(tok());

      MaterialBuildSpec_x spec;
      spec.x = x;
      spec.finalized = true;

      GetConfigInstance()->matBuildX[matName] = spec;
    }

    if (cmd == fMatAddMass) {
      G4Tokenizer tok(value);
      auto mat = tok();
      auto el = tok();
      auto fr = tok();

      GetConfigInstance()->matBuild[mat].useAtoms = false;
      GetConfigInstance()->matBuild[mat].mass.push_back({el, std::stod(fr)});

      return;
    }

    if (cmd == fMatFinalize) {
      G4Tokenizer tok(value);
      auto mat = tok();
      GetConfigInstance()->matBuild[mat].finalized = true;

      return;
    }

    // --------------------------------------------------------
    // SOURCE (Physics)
    if (cmd == fSourceType) {
      GetConfigInstance()->sourceType = (value == "gun")
                            ? GeantCore::Models::Experiment::SourceType::Gun
                            : GeantCore::Models::Experiment::SourceType::Decay;

      return;
    }

    if (cmd == fGunParticle) {
      GetConfigInstance()->gun.particle = value;
      return;
    }

    if (cmd == fGunEnergy) {
      GetConfigInstance()->gun.energy = fGunEnergy->GetNewDoubleValue(value);
      return;
    }

    if (cmd == fGunPos) {
      GetConfigInstance()->gun.pos = fGunPos->GetNew3VectorValue(value);
      return;
    }

    if (cmd == fGunDir) {
      G4Tokenizer tok(value);
      auto dx = tok();
      auto dy = tok();
      auto dz = tok();

      GetConfigInstance()->gun.dir =
          G4ThreeVector(std::stod(dx), std::stod(dy), std::stod(dz)).unit();

      return;
    };
  }
  void Release() override {
    delete fGunDir;
    delete fGunPos;
    delete fGunEnergy;
    delete fGunParticle;
    delete fSourceType;

    delete fMatFinalize;
    delete fMatCreateX;
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

#define region Properties
private:
  BaseExperimentConfig* GetConfigInstance() {
    if (fCfg ==  nullptr) {
      fCfg = std::make_unique<BaseExperimentConfig>();
    }
    return fCfg.get();
  };
#define endregion

#pragma region Fields
private:
  std::unique_ptr<GeantCore::Models::Experiment::BaseExperimentConfig> fCfg;

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
  G4UIcommand *fMatCreateX = nullptr;

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