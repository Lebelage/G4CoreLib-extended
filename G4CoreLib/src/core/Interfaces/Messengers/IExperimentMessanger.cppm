module;

#include "G4UImessenger.hh"

export module GeantCore.Core.Interfaces.IExperimentMessenger;

export namespace GeantCore::Core::Interfaces {
class IExperimentMessenger : public G4UImessenger {
public:
  ~IExperimentMessenger() override = default;

  void SetNewValue(G4UIcommand *cmd, G4String value) override = 0;
  virtual void Release() = 0;
};
} // namespace GeantCore::Core::Interfaces