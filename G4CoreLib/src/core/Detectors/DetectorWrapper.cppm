module;
#include <functional>
#include <G4Step.hh>
#include <G4VUserDetectorConstruction.hh>
#include <memory>
export module GeantCore.Core.Detectors.DetectorWrapper;
import GeantCore.Core.Concepts.DetectorConstructionConcept;
export namespace GeantCore::Core::Detectors {
    using namespace GeantCore::Core::Concepts;

    class DetectorWrapper : public G4VUserDetectorConstruction {
    private:
        std::unique_ptr<G4VUserDetectorConstruction> currentBuilder;
        std::function<void()> analyzer;

    public:
        template<DetectorConstructionConcept Detector>
        void SetBuilder(std::unique_ptr<Detector> builder) {
            analyzer = [ptr = builder.get()]() { ptr->Analyze(); };
            currentBuilder = std::move(builder);
        }

        G4VPhysicalVolume *Construct() override {
            return BuildWorld();
        }

        G4VPhysicalVolume *BuildWorld() {
            if (!currentBuilder) {
                return nullptr;
            }
            return currentBuilder->Construct();
        }

        void ConstructSDandField() override
        {
            if (currentBuilder) {
                currentBuilder->ConstructSDandField();
            }
        }

        void Analyze() {
            if (analyzer) {
                analyzer();
            }
        }
    };
}
