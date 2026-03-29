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
        std::function<void(const G4Step*)> analyzer;

    public:
        template<DetectorConstructionConcept Detector>
        void SetBuilder(std::unique_ptr<Detector> builder) {
            analyzer = [ptr = builder.get()](const G4Step* step) { ptr->Analyze(step); };
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

        void Analyze(const G4Step* step) {
            if (analyzer) {
                analyzer(step);
            }
        }
    };
}
