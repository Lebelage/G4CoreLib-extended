module;
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserRunAction.hh"
export module GeantCore.Core.Actions.BaseRunAction;
import GeantCore.Core.Interfaces.IRunAction;
export namespace GeantCore::Core::Actions {
using namespace GeantCore::Core::Interfaces;

class BaseRunAction : IRunAction {
#pragma region Constructor/Destructor
public:
  BaseRunAction() {};
  ~BaseRunAction() {};

  BaseRunAction(const BaseRunAction &) = delete;
  BaseRunAction &operator=(const BaseRunAction &) = delete;

  BaseRunAction(const BaseRunAction &&) = delete;
  BaseRunAction &operator=(const BaseRunAction &&) = delete;
#pragma endregion

#pragma region Methods
  void BeginOfRunAction(const G4Run *) override {

  };
  void EndOfRunAction(const G4Run *) override {

  };
#pragma endregion
};
} // namespace GeantCore::Core::Actions