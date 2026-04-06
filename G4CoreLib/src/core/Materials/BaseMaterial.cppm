module;
#include "G4NistManager.hh"
#include <string>
#include "G4SystemOfUnits.hh"
#include <unordered_map>
export module GeantCore.Core.Materials.BaseMaterials;
import GeantCore.Core.Interfaces.IMaterials;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Materials.MaterialsConstants;
import GeantCore.Core.Materials.ExtendedG4Material;
export namespace GeantCore::Core::Materials {
    using namespace GeantCore::Core::Interfaces;
    using namespace GeantCore::Models::Experiment;

    class BaseMaterials {
#pragma region Constructor/Destructor

    public:
        explicit BaseMaterials(const BaseExperimentConfig &config) : fCfg{config} {
            fNist = G4NistManager::Instance();
        };
#pragma endregion

#pragma region Methods

    public:
        std::optional<ExtendedG4Material *> Get(const G4String &name) {
            const std::string key = name;

            if (auto it = fCache.find(key); it != fCache.end())
                return it->second;

            if (name.rfind("G4_", 0) == 0) {
                auto *m = fNist->FindOrBuildMaterial(name, true);
                if (!m)
                    G4Exception("Materials", "NoNIST", FatalException,
                                "Cannot build NIST material.");

                ExtendedG4Material* mat = new ExtendedG4Material(m, 0, 0, false);
                fCache[key] = mat;
                return mat;
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
        ExtendedG4Material *BuildFromSpec(const std::string &name,
                                         const MaterialBuildSpec_x &spec) {
            if (!spec.finalized) {
                G4Exception("Materials", "NotFinalized", FatalException,
                            ("Material not finalized: " + name).c_str());
            }

            double x = spec.x;
            double density = 0.0;
            std::vector<std::pair<std::string, double> > components;

            if (name.find("AlGaN") != std::string::npos || name.find("algan") != std::string::npos) {
                double m_total = x * MaterialsConstants::M_Al + (1.0 - x) * MaterialsConstants::M_Ga +
                                 MaterialsConstants::M_N;
                density = x * MaterialsConstants::DENS_AlN + (1.0 - x) * MaterialsConstants::DENS_GaN;

                components.push_back({"Al", (x * MaterialsConstants::M_Al) / m_total});
                components.push_back({"Ga", ((1.0 - x) * MaterialsConstants::M_Ga) / m_total});
                components.push_back({"N", MaterialsConstants::M_N / m_total});
            } else if (name.find("InGaN") != std::string::npos || name.find("ingan") != std::string::npos) {
                double m_total = x * MaterialsConstants::M_In + (1.0 - x) * MaterialsConstants::M_Ga +
                                 MaterialsConstants::M_N;
                density = x * MaterialsConstants::DENS_InN + (1.0 - x) * MaterialsConstants::DENS_GaN;

                components.push_back({"In", (x * MaterialsConstants::M_In) / m_total});
                components.push_back({"Ga", ((1.0 - x) * MaterialsConstants::M_Ga) / m_total});
                components.push_back({"N", MaterialsConstants::M_N / m_total});
            } else {
                G4Exception("Materials", "UnknownAlloy", FatalException,
                            ("Cannot build alloy from x for: " + name + ". Name must contain AlGaN or InGaN.").c_str());
            }

            // Собираем сам G4Material
            G4Material *mat = new G4Material(name, density * (g / cm3), components.size());

            for (const auto &[elName, fraction]: components) {
                G4Element *el = fNist->FindOrBuildElement(elName, true);
                if (!el) {
                    G4Exception("Materials", "NoElement", FatalException,
                                ("Cannot build element: " + elName).c_str());
                }
                mat->AddElement(el, fraction);
            }

            ExtendedG4Material *material = new ExtendedG4Material(mat, x, CalculateEg(name, x), true);

            return material;
        }

        ExtendedG4Material *BuildFromSpec(const std::string &name,
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
                auto *mat = new G4Material(name, spec.density, (int) spec.mass.size());
                for (const auto &m: spec.mass) {
                    auto *el = fNist->FindOrBuildElement(m.element, true);
                    if (!el)
                        G4Exception("Materials", "NoElement", FatalException,
                                    "Cannot build element.");
                    mat->AddElement(el, m.massFraction); // mass fraction
                }

                float Eg = 0.f;

                if (name == "GaN")
                    Eg = MaterialsConstants::EG_GAN;

                ExtendedG4Material *material = new ExtendedG4Material(mat,0, Eg, false);

                return material;
            };
        }

        float CalculateEg(std::string matName, float x) {
            // Если передали x за пределами [0; 1], логично обрезать его или выдать предупреждение
            if (x < 0.0f) x = 0.0f;
            if (x > 1.0f) x = 1.0f;

            // Расчет для барьерного слоя AlGaN
            if (matName.find("AlGaN") != std::string::npos || matName.find("algan") != std::string::npos) {
                return x * MaterialsConstants::EG_ALN +
                       (1.0f - x) * MaterialsConstants::EG_GAN -
                       MaterialsConstants::B_ALGAN * x * (1.0f - x);
            }
            // Расчет для активной зоны InGaN
            else if (matName.find("InGaN") != std::string::npos || matName.find("ingan") != std::string::npos) {
                return x * MaterialsConstants::EG_INN +
                       (1.0f - x) * MaterialsConstants::EG_GAN -
                       MaterialsConstants::B_INGAN * x * (1.0f - x);
            }
            // Если передали чистый GaN (например, базовый слой или контакты)
            else if (matName.find("GaN") != std::string::npos || matName.find("gan") != std::string::npos) {
                return MaterialsConstants::EG_GAN;
            }

            // Защита от ошибок: если материал не распознан
            std::cerr << "[ОШИБКА] Неизвестный материал для расчета Eg: " << matName << std::endl;
            return 0.0f;
        }
#pragma endregion

#pragma region Fields
        const BaseExperimentConfig &fCfg;

        G4NistManager *fNist = nullptr;
        std::unordered_map<std::string, ExtendedG4Material*> fCache;
#pragma endregion
    };
} // namespace GeantCore::Core::Materials
