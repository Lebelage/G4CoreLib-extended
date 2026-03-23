module;
#include "G4UserSteppingAction.hh"
export module GeantCore.Core.Actions.BaseSteppingAction;
import GeantCore.Core.Interfaces.ISteppingAction;
export namespace GeantCore::Core::Actions {
using namespace GeantCore::Core::Interfaces;

class BaseSteppingAction : ISteppingAction {
#pragma region Constructor/Destructor
public:
  BaseSteppingAction() {};
  ~BaseSteppingAction() {};

  BaseSteppingAction(const BaseSteppingAction &) = delete;
  BaseSteppingAction &operator=(const BaseSteppingAction &) = delete;

  BaseSteppingAction(const BaseSteppingAction &&) = delete;
  BaseSteppingAction &operator=(const BaseSteppingAction &&) = delete;
#pragma endregion

#pragma region Methods
  void UserSteppingAction(const G4Step *step) override {

  };
#pragma endregion
};
} // namespace GeantCore::Core::Actions