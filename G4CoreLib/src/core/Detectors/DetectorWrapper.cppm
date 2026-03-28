module;
#include <G4VUserDetectorConstruction.hh>
#include <memory>
export module GeantCore.Core.Detectors.DetectorWrapper;
import GeantCore.Models.Experiment.ExperimentConfig;
import GeantCore.Core.Concepts.DetectorConstructionConcept;
export namespace GeantCore::Core::Detectors {
    using namespace GeantCore::Core::Concepts;

    class DetectorWrapper : public G4VUserDetectorConstruction {
    private:
        GeantCore::Models::Experiment::BaseExperimentConfig &config;
        std::unique_ptr<G4VUserDetectorConstruction> currentBuilder;

    public:
        DetectorWrapper(GeantCore::Models::Experiment::BaseExperimentConfig &cfg) : config(cfg) {
        }

        template<DetectorConstructionConcept Detector>
        void SetBuilder(std::unique_ptr<Detector> builder) {
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
    };
}
