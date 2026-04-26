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
import GeantCore.Core.Materials.ExtendedG4Material;

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
                eventCount,
                GetDetectorThickness(),
                binWidth,
                fGlobalZProfile,
                fProfileMutex
            );

            G4SDManager::GetSDMpointer()->AddNewDetector(layerSD);

            SetSensitiveDetector("LayerLV", layerSD, true);

            G4cout << "[SD] LayerSensor attached to LayerLV" << G4endl;
        }

        void Analyze() {
            using json = nlohmann::json;

            unsigned long long absCount = absorbedCount.load(std::memory_order_relaxed);
            unsigned long long refCount = reflectedCount.load(std::memory_order_relaxed);
            unsigned long long totalEvents = eventCount.load(std::memory_order_relaxed);

            if (totalEvents == 0) {
                totalEvents = 1;
            }

            AlGanModel model{
                static_cast<uint64_t>(absCount),
                static_cast<uint64_t>(refCount)
            };

            json j = model;

            auto &inst = PostProcessManager::getInstance();

            inst.PostProcess(
                std::move(fGlobalZProfile),
                std::move(fLayersMaterialsInfo),
                totalEvents

            );

            auto str = inst.SerializeLayersToJson();

            j["energy_profile"] = json::parse(str);
            j["events"] = totalEvents;
            j["absorbed"] = absCount;
            j["reflected"] = refCount;

            FileProvider::CreateAndWriteToExperimentFile(
                "AlGaNExerimentInfo.json",
                j.dump(4)
            );
        }

        G4double GetTotalThickness() const {
            return fTotalZ;
        }

        G4double GetStackTopZ() const {
            return fStackTopZ;
        }

        G4double GetDetectorThickness() const {
            return fDetectorZ;
        }

    private:
        G4VPhysicalVolume *BuildStack() const {
            auto *worldMat = mats->Get(fCfg->worldMaterial).value()->GetG4Material();

            // 1. World
            auto halfWorld = fCfg->worldSize / 2.0;

            auto *solidWorld = new G4Box(
                "World",
                halfWorld,
                halfWorld,
                halfWorld
            );

            auto *logicWorld = new G4LogicalVolume(
                solidWorld,
                worldMat,
                "WorldLV"
            );

            auto *physWorld = new G4PVPlacement(
                nullptr,
                {},
                logicWorld,
                "WorldPV",
                nullptr,
                false,
                0
            );

            // 2. Считаем толщины источника и детектора
            double totalSourceZ = 0.0;
            double totalDetectorZ = 0.0;

            for (const auto &L: fCfg->layers) {
                if (L.material == "Ni63_Source") {
                    totalSourceZ += L.thickness;
                } else {
                    totalDetectorZ += L.thickness;
                }
            }

            // Источник: 0 -> +Z
            // Детектор: 0 -> -Z
            fTotalZ = totalSourceZ + totalDetectorZ;
            fDetectorZ = totalDetectorZ;
            fStackTopZ = fCfg->stackPos.z() + totalSourceZ;

            // 3. Stack-контейнер стоит центром ровно в fCfg->stackPos
            // Но должен покрывать и +Z, и -Z от локального нуля
            double stackHalfZ = std::max(totalSourceZ, totalDetectorZ);

            auto *solidStack = new G4Box(
                "StackSolid",
                fCfg->stackX / 2.0,
                fCfg->stackY / 2.0,
                stackHalfZ
            );

            auto *logicStack = new G4LogicalVolume(
                solidStack,
                worldMat,
                "StackLV"
            );

            new G4PVPlacement(
                nullptr,
                fCfg->stackPos,
                logicStack,
                "StackPV",
                logicWorld,
                false,
                0
            );

            // 4. Интерфейс источник/детектор находится в локальном Z=0 StackLV
            double zCursorSource = 0.0; // вверх: 0 -> +Z
            double zCursorDetector = 0.0; // вниз: 0 -> -Z

            int copyNo = 0;

            for (const auto &L: fCfg->layers) {
                auto extMat = mats->Get(L.material).value();
                auto *mat = extMat->GetG4Material();

                bool isSource = (L.material == "Ni63_Source");

                auto *solidLayer = new G4Box(
                    isSource ? "Ni63Solid" : "LayerSolid",
                    fCfg->stackX / 2.0,
                    fCfg->stackY / 2.0,
                    L.thickness / 2.0
                );

                auto *logicLayer = new G4LogicalVolume(
                    solidLayer,
                    mat,
                    isSource ? "Ni63LV" : "LayerLV"
                );

                auto *limits = new G4UserLimits(
                    MaterialsConstants::MAX_STEP_LIMIT * nm
                );

                logicLayer->SetUserLimits(limits);

                double zPos = 0.0;

                if (isSource) {
                    // Радиоизотоп / источник: от 0 в +Z
                    zPos = zCursorSource + L.thickness / 2.0;
                    zCursorSource += L.thickness;
                } else {
                    // Детектор: от 0 в -Z
                    zPos = zCursorDetector - L.thickness / 2.0;
                    zCursorDetector -= L.thickness;
                }

                new G4PVPlacement(
                    nullptr,
                    G4ThreeVector(0, 0, zPos),
                    logicLayer,
                    isSource ? "Ni63PV" : "LayerPV",
                    logicStack,
                    false,
                    copyNo
                );

                fLayersMaterialsInfo[static_cast<uint8_t>(copyNo)] = *extMat;

                copyNo++;
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
        mutable G4double fDetectorZ = 0.0;

        std::atomic<unsigned long long> absorbedCount{0};
        std::atomic<unsigned long long> reflectedCount{0};
        std::atomic<unsigned long long> eventCount{0};

        std::unique_ptr<BaseMaterials> mats;

        std::vector<LayerInfo> fGlobalZProfile;
        std::mutex fProfileMutex;

        mutable std::unordered_map<uint8_t, ExtendedG4Material> fLayersMaterialsInfo;
#pragma endregion
    };
} // namespace GeantCore::Core::Detectors
