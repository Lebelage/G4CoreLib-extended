module;
#include <G4VUserDetectorConstruction.hh>

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Types.hh"
#include "G4UserLimits.hh"
#include "G4VPhysicalVolume.hh"

#include <memory>
export module GeantCore.Core.Detectors.DetectorConstruction;
import GeantCore.Core.Interfaces.IDetectorConstruction;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Interfaces.IMaterials;
import GeantCore.Core.Materials.BaseMaterials;
export namespace GeantCore::Core::Detectors {
    using namespace GeantCore::Core::Interfaces;
    using namespace GeantCore::Models::Experiment;
    using namespace GeantCore::Core::Materials;

    class BaseDetectorConstruction : public G4VUserDetectorConstruction {
#pragma region Constructors/Destructor

    public:
        BaseDetectorConstruction(
            GeantCore::Models::Experiment::BaseExperimentConfig &&config)
            : fCfg{config} {
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

        G4VPhysicalVolume *BuildWorld() const  {
            if (fCfg.type == ExpType::Stack)
                return BuildStack();

            // fallback
            auto worldMat =
                    G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

            auto solidWorld = new G4Box("World", 1 * mm, 1 * mm, 1 * mm);
            auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");

            return new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false,
                                     0);
        };

        G4double GetTotalThickness() const { return fTotalZ; }
        G4double GetStackTopZ() const { return fStackTopZ; }

    private:
        G4VPhysicalVolume *BuildStack() const {
            std::unique_ptr<IMaterials> mats = std::make_unique<BaseMaterials>(fCfg);

            auto *worldMat = mats->Get(fCfg.worldMaterial).value();

            // World is a cube worldSize^3
            auto half = fCfg.worldSize / 2.0;
            auto *solidWorld = new G4Box("World", half, half, half);
            auto *logicWorld = new G4LogicalVolume(solidWorld, worldMat, "WorldLV");
            auto *physWorld = new G4PVPlacement(nullptr, {}, logicWorld, "WorldPV",
                                                nullptr, false, 0);

            if (fCfg.layers.empty()) {
                G4Exception("DetectorConstruction", "NoLayers", FatalException,
                            "No layers. Use /exp/layer/add.");
            }

            double totalZ = 0.0;
            for (auto &L: fCfg.layers)
                totalZ += L.thickness;

            fTotalZ = totalZ;
            fStackTopZ = fCfg.stackPos.z() + totalZ / 2.0;

            // Stack container (vacuum)
            auto *solidStack =
                    new G4Box("StackSolid", fCfg.stackX / 2, fCfg.stackY / 2, totalZ / 2);
            auto *logicStack = new G4LogicalVolume(solidStack, worldMat, "StackLV");
            new G4PVPlacement(nullptr, fCfg.stackPos, logicStack, "StackPV", logicWorld,
                              false, 0);

            // Place layers along +Z from top surface
            double zTop = totalZ / 2.0;
            double zCursor = zTop;

            int copyNo = 0;

            std::vector<std::string> layers;

            for (const auto &L: fCfg.layers) {
                auto *mat = mats->Get(L.material).value();

                auto *solidLayer = new G4Box("LayerSolid", fCfg.stackX / 2,
                                             fCfg.stackY / 2, L.thickness / 2);
                auto *logicLayer = new G4LogicalVolume(solidLayer, mat, "LayerLV");

                /// limits
                auto *limits = new G4UserLimits();
                limits->SetMaxAllowedStep(1 * nm);
                logicLayer->SetUserLimits(limits);

                zCursor -= L.thickness / 2.0;
                new G4PVPlacement(nullptr, {0, 0, zCursor}, logicLayer, "LayerPV",
                                  logicStack, false, copyNo);
                zCursor -= L.thickness / 2.0;

                copyNo++;

                layers.push_back(L.material);
            }

            return physWorld;
        };

#pragma endregion

#pragma region Fields

    private:
        BaseExperimentConfig &fCfg;
        mutable G4double fTotalZ = 0.0;
        mutable G4double fStackTopZ = 0.0;
#pragma endregion
    };
} // namespace GeantCore::Core::Detectors
