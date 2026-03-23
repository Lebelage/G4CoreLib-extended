module;
#include "G4UserSteppingAction.hh"
export module GeantCore.Core.Interfaces.ISteppingAction;
export namespace GeantCore::Core::Interfaces {
class ISteppingAction : public G4UserSteppingAction {
public:
  virtual ~ISteppingAction() = default;

  virtual void UserSteppingAction(const G4Step *step) = 0;
};
} // namespace GeantCore::Core::Interfaces