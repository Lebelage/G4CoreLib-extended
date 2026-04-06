module;
#include <G4EventManager.hh>
#include <G4Step.hh>
#include <G4VUserDetectorConstruction.hh>

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Types.hh"
#include "G4UserLimits.hh"
#include "G4VPhysicalVolume.hh"
#include <G4SDManager.hh>

#include <memory>
#include <nlohmann/json.hpp>
#include <CLHEP/Utility/memory.h>
export module GeantCore.Core.Detectors.DetectorConstruction;
import GeantCore.Core.Interfaces.IDetectorConstruction;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Interfaces.IMaterials;
import GeantCore.Core.Materials.BaseMaterials;
import GeantCore.Core.SensitiveDetectors.BaseSD;
import GeantCore.Models.AlGaNModel;
import GeantCore.Utils.FileProvider;
import GeantCore.Core.Materials.MaterialsConstants;
import GeantCore.Core.PostProcessManager;

using json = nlohmann::json;
export namespace GeantCore::Core::Detectors {
    using namespace GeantCore::Core::Interfaces;
    using namespace GeantCore::Models::Experiment;
    using namespace GeantCore::Core::Materials;
    using namespace GeantCore::Core::SensitiveDetectors;
    using namespace GeantCore::Models;
    using namespace GeantCore::Utils::FileProvider;

    class BaseDetectorConstruction : public G4VUserDetectorConstruction {
#pragma region Constructors/Destructor

    public:
        BaseDetectorConstruction(
            std::shared_ptr<BaseExperimentConfig> config)
            : fCfg{std::move(config)} {
            mats = std::make_unique<BaseMaterials>(*fCfg);
        };

        ~BaseDetectorConstruction() override {
        };

        BaseDetectorConstruction(const BaseDetectorConstruction &) = delete;

        BaseDetectorConstruction &
        operator=(const BaseDetectorConstruction &) = delete;

        BaseDetectorConstruction(const BaseDetectorConstruction &&) = delete;

        BaseDetectorConstruction &
        operator=(const BaseDetectorConstruction &&) = delete;
#pragma endregion

#pragma region Methods

    public:
        G4VPhysicalVolume *Construct() override { return BuildWorld(); };

        G4VPhysicalVolume *BuildWorld() const {
            if (fCfg->type == ExpType::Stack)
                return BuildStack();

            // fallback
            auto worldMat =
                    G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

            auto solidWorld = new G4Box("World", 1 * mm, 1 * mm, 1 * mm);
            auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");

            return new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false,
                                     0);
        };

        void ConstructSDandField() override {

            G4double binWidth = MaterialsConstants::MAX_STEP_LIMIT * nm;

            auto *layerSD = new BaseSD(
                "LayerSensor",
                absorbedCount,
                reflectedCount,
                GetStackTopZ(),
                binWidth,
                fGlobalZProfile,
                fProfileMutex
            );

            G4SDManager::GetSDMpointer()->AddNewDetector(layerSD);
            SetSensitiveDetector("LayerLV", layerSD, true);
        }

        void Analyze() {
            AlGanModel model{
                static_cast<uint16_t>(absorbedCount.load()),
                static_cast<uint16_t>(reflectedCount.load())
            };
            json j = model;

            // === Сериализация JSON для профиля слоев ===
            json profileArray = json::array();

            // Проходим по вектору структур LayerInfo
            for (const auto& layer : fGlobalZProfile) {
                profileArray.push_back({
                    {"layerID", layer.layerID},
                    {"layerName", layer.layerName},
                    {"z_depth_nm", layer.z_depth / nm},
                    {"edep_MeV", layer.Edep},
                    {"ehp_count", layer.EHP_count}
                });
            }

            // Добавляем массив профиля в основной JSON
            j["energy_profile"] = profileArray;

            // Записываем в файл (std::move убран из j.dump, так как dump() возвращает std::string по значению)
            FileProvider::CreateAndWriteToExperimentFile("AlGaNExerimentInfo.json", j.dump(4));
        };

        G4double GetTotalThickness() const {
            return fTotalZ;
        }

        G4double GetStackTopZ() const {
            return fStackTopZ;
        }

    private:
        G4VPhysicalVolume *BuildStack() const {

            auto *worldMat = mats->Get(fCfg->worldMaterial).value()->GetG4Material();

            // World is a cube worldSize^3
            auto half = fCfg->worldSize / 2.0;
            auto *solidWorld = new G4Box("World", half, half, half);
            auto *logicWorld = new G4LogicalVolume(solidWorld, worldMat, "WorldLV");
            auto *physWorld = new G4PVPlacement(nullptr, {}, logicWorld, "WorldPV",
                                                nullptr, false, 0);

            if (fCfg->layers.empty()) {
                G4Exception("DetectorConstruction", "NoLayers", FatalException,
                            "No layers. Use /exp/layer/add.");
            }

            double totalZ = 0.0;
            for (auto &L: fCfg->layers)
                totalZ += L.thickness;

            fTotalZ = totalZ;
            fStackTopZ = fCfg->stackPos.z() + totalZ / 2.0;

            // Stack container (vacuum)
            auto *solidStack =
                    new G4Box("StackSolid", fCfg->stackX / 2, fCfg->stackY / 2, totalZ / 2);
            auto *logicStack = new G4LogicalVolume(solidStack, worldMat, "StackLV");
            new G4PVPlacement(nullptr, fCfg->stackPos, logicStack, "StackPV", logicWorld,
                              false, 0);

            // Place layers along +Z from top surface
            double zTop = totalZ / 2.0;
            double zCursor = zTop;

            int copyNo = 0;

            //std::vector<std::string> layers;

            for (const auto &L: fCfg->layers) {
                auto *mat = mats->Get(L.material).value()->GetG4Material();

                auto *solidLayer = new G4Box("LayerSolid", fCfg->stackX / 2,
                                             fCfg->stackY / 2, L.thickness / 2);
                auto *logicLayer = new G4LogicalVolume(solidLayer, mat, "LayerLV");

                /// limits
                auto *limits = new G4UserLimits();
                limits->SetMaxAllowedStep(MaterialsConstants::MAX_STEP_LIMIT * nm);
                logicLayer->SetUserLimits(limits);

                zCursor -= L.thickness / 2.0;
                new G4PVPlacement(nullptr, {0, 0, zCursor}, logicLayer, "LayerPV",
                                  logicStack, false, copyNo);
                zCursor -= L.thickness / 2.0;

                copyNo++;

                //layers.push_back(L.material);
            }

            return physWorld;
        };

#pragma endregion


#pragma region Fields

    private
    :
        std::shared_ptr<BaseExperimentConfig> fCfg;
        mutable G4double fTotalZ = 0.0;
        mutable G4double fStackTopZ = 0.0;

        std::atomic<unsigned long long> absorbedCount;
        std::atomic<unsigned long long> reflectedCount;

        std::unique_ptr<BaseMaterials> mats;

        std::vector<LayerInfo> fGlobalZProfile;
        std::mutex fProfileMutex;
#pragma endregion
    };
} // namespace GeantCore::Core::Detectors
