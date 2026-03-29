module;
#include <G4Step.hh>
#include <memory>
export module GeantCore.Core.EventManager;
import GeantCore.Utils.Event;
import GeantCore.Models.Experiment.ExperimentConfig;
export namespace GeantCore::Core {
    using namespace GeantCore::Utils;
    using namespace GeantCore::Models::Experiment;

    class EventManager {
    public:
        static Event<std::shared_ptr<BaseExperimentConfig> > &GetGeometryUpdatedEvent() { return GeometryUpdated; };
        static Event<const G4Step *>& GetSteppingAction() { return SteppingAction; };
    private:
        inline static Event<std::shared_ptr<BaseExperimentConfig> > GeometryUpdated;
        inline static Event<const G4Step *> SteppingAction;
    };
} // namespace GeantCore::Core
