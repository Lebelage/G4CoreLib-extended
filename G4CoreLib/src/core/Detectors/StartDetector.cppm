
module;
#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4PVPlacement.hh>
#include <G4Step.hh>

export module GeantCore.Core.Detectors.StartDetector;

export namespace GeantCore::Core::Detectors {
    class StartDetector : public G4VUserDetectorConstruction {

#pragma region Constructor Declarations
        public:
        StartDetector() {}
        ~StartDetector() override {
        };

        StartDetector(const StartDetector &) = delete;

        StartDetector &
        operator=(const StartDetector &) = delete;

        StartDetector(const StartDetector &&) = delete;

        StartDetector &
        operator=(const StartDetector &&) = delete;
#pragma endregion

    public:
        G4VPhysicalVolume *Construct() override {
            auto* nist = G4NistManager::Instance();

            auto* worldMat = nist->FindOrBuildMaterial("G4_Galactic");

            auto* solidWorld = new G4Box("World", 1, 1, 1);
            auto* logicWorld = new G4LogicalVolume(
                solidWorld,
                worldMat,
                "WorldLV"
            );

            auto* physWorld = new G4PVPlacement(
                nullptr,
                G4ThreeVector(),
                logicWorld,
                "WorldPV",
                nullptr,
                false,
                0
            );

            return physWorld;

        };
        void Analyze(const G4Step* step) {};
    };
}
