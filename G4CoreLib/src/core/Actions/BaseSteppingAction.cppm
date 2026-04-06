module;
#include "G4UserSteppingAction.hh"
#include <G4Types.hh>
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4TouchableHandle.hh"
#include "G4VPhysicalVolume.hh"
export module GeantCore.Core.Actions.BaseSteppingAction;
import GeantCore.Core.Interfaces.ISteppingAction;
import GeantCore.Core.Interfaces.IDetectorConstruction;
import GeantCore.Core.EventManager;
export namespace GeantCore::Core::Actions {
    using namespace GeantCore::Core::Interfaces;

    class BaseSteppingAction : public ISteppingAction {
#pragma region Constructor/Destructor

    public:
        BaseSteppingAction() {
        }

        ~BaseSteppingAction() {
        };

        BaseSteppingAction(const BaseSteppingAction &) = delete;

        BaseSteppingAction &operator=(const BaseSteppingAction &) = delete;

#pragma endregion

#pragma region Methods
        void UserSteppingAction(const G4Step *step) override {

        };

#pragma endregion
#pragma region Fields
#pragma endregion
    };
} // namespace GeantCore::Core::Actions
