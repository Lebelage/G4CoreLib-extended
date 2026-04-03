module;
#include "G4NistManager.hh"
#include <string>
#include "G4SystemOfUnits.hh"
#include <unordered_map>
export module GeantCore.Core.Materials.BaseMaterials;
import GeantCore.Core.Interfaces.IMaterials;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Materials.MaterialsConstatnts;
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

    // ДОБАВЛЕНО: Проверяем материалы, заданные через x
    if (auto it = fCfg.matBuildX.find(key); it != fCfg.matBuildX.end()) {
      auto *m = BuildFromSpec(key, it->second);
      fCache[key] = m;
      return m;
    }

    G4Exception("Materials", "UnknownMaterial", FatalException,
                ("Unknown material: " + key).c_str());

    return std::nullopt;
  };

private:
  G4Material* BuildFromSpec(const std::string &name,
                            const MaterialBuildSpec_x &spec) {
    if (!spec.finalized) {
      G4Exception("Materials", "NotFinalized", FatalException,
                  ("Material not finalized: " + name).c_str());
    }

    double x = spec.x;
    double density = 0.0;
    std::vector<std::pair<std::string, double>> components;

    if (name.find("AlGaN") != std::string::npos || name.find("algan") != std::string::npos) {
      double m_total = x * MaterialsConstants::M_Al + (1.0 - x) * MaterialsConstants::M_Ga + MaterialsConstants::M_N;
      density = x * MaterialsConstants::DENS_AlN + (1.0 - x) * MaterialsConstants::DENS_GaN;

      components.push_back({"Al", (x * MaterialsConstants::M_Al) / m_total});
      components.push_back({"Ga", ((1.0 - x) * MaterialsConstants::M_Ga) / m_total});
      components.push_back({"N",  MaterialsConstants::M_N / m_total});

    } else if (name.find("InGaN") != std::string::npos || name.find("ingan") != std::string::npos) {
      double m_total = x * MaterialsConstants::M_In + (1.0 - x) * MaterialsConstants::M_Ga + MaterialsConstants::M_N;
      density = x * MaterialsConstants::DENS_InN + (1.0 - x) * MaterialsConstants::DENS_GaN;

      components.push_back({"In", (x * MaterialsConstants::M_In) / m_total});
      components.push_back({"Ga", ((1.0 - x) * MaterialsConstants::M_Ga) / m_total});
      components.push_back({"N",  MaterialsConstants::M_N / m_total});

    } else {
      G4Exception("Materials", "UnknownAlloy", FatalException,
                  ("Cannot build alloy from x for: " + name + ". Name must contain AlGaN or InGaN.").c_str());
    }

    // Собираем сам G4Material
    G4Material* mat = new G4Material(name, density * (g/cm3), components.size());

    for (const auto& [elName, fraction] : components) {
      G4Element* el = fNist->FindOrBuildElement(elName, true);
      if (!el) {
        G4Exception("Materials", "NoElement", FatalException,
                    ("Cannot build element: " + elName).c_str());
      }
      mat->AddElement(el, fraction);
    }

    return mat;

  }

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