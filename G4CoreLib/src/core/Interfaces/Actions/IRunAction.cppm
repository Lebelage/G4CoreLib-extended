module;
#include "G4UserRunAction.hh"
export module GeantCore.Core.Interfaces.IRunAction;
export namespace GeantCore::Core::Interfaces {
class IRunAction : public G4UserRunAction {
public:
  virtual ~IRunAction() = default;

  virtual void BeginOfRunAction(const G4Run *) = 0;
  virtual void EndOfRunAction(const G4Run *) = 0;
};
} // namespace GeantCore::Core::Interfaces