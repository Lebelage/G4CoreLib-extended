module;
#include "G4NistManager.hh"
#include <string>
#include <unordered_map>
export module GeantCore.Core.Materials.BaseMaterials;
import GeantCore.Core.Interfaces.IMaterials;
import GeantCore.Models.Experiment.ExperimentConfig;
export namespace GeantCore::Core::Materials {
using namespace GeantCore::Core::Interfaces;
using namespace GeantCore::Models::Experiment;
class BaseMaterials : public IMaterials {
#pragma region Constructor/Destructor
public:
  explicit BaseMaterials(const BaseExperimentConfig &config) : fCfg{config} {
    fNist = G4NistManager::Instance();
  };
#pragma endregion

#pragma region Methods
public:
  std::optional<G4Material *> Get(const G4String &name) override {
    const std::string key = name;

    if (auto it = fCache.find(key); it != fCache.end())
      return it->second;

    if (name.rfind("G4_", 0) == 0) {
      auto *m = fNist->FindOrBuildMaterial(name, true);
      if (!m)
        G4Exception("Materials", "NoNIST", FatalException,
                    "Cannot build NIST material.");
      fCache[key] = m;
      return m;
    }

    if (auto it = fCfg.matBuild.find(key); it != fCfg.matBuild.end()) {
      auto *m = BuildFromSpec(key, it->second);
      fCache[key] = m;
      return m;
    }

    G4Exception("Materials", "UnknownMaterial", FatalException,
                ("Unknown material: " + key).c_str());

    return std::nullopt;
  };

private:
  G4Material *BuildFromSpec(const std::string &name,
                            const MaterialBuildSpec &spec) {
    if (!spec.finalized) {
      G4Exception("Materials", "NotFinalized", FatalException,
                  ("Material not finalized: " + name).c_str());
    }
    if (spec.density <= 0) {
      G4Exception("Materials", "BadDensity", FatalException,
                  ("Bad density for material: " + name).c_str());
    }

    if (spec.useAtoms) {
      // auto *mat = new G4Material(name, spec.density, (int)spec.atoms.size());
      // for (const auto &a : spec.atoms)
      // {
      //     auto *el = fNist->FindOrBuildElement(a.element, true);
      //     if (!el)
      //         G4Exception("Materials", "NoElement", FatalException, "Cannot
      //         build element.");
      //     mat->AddElement(el, a.natoms); // stoichiometry
      // }
      // return mat;

      return nullptr; // atoms is not usable
    } else {
      auto *mat = new G4Material(name, spec.density, (int)spec.mass.size());
      for (const auto &m : spec.mass) {
        auto *el = fNist->FindOrBuildElement(m.element, true);
        if (!el)
          G4Exception("Materials", "NoElement", FatalException,
                      "Cannot build element.");
        mat->AddElement(el, m.massFraction); // mass fraction
      }
      return mat;
    };
  }
#pragma endregion

#pragma region Fields
  const BaseExperimentConfig &fCfg;

  G4NistManager *fNist = nullptr;
  std::unordered_map<std::string, G4Material *> fCache;
#pragma endregion
};
} // namespace GeantCore::Core::Materials