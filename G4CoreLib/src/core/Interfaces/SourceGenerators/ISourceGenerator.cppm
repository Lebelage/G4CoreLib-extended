module;
#include "G4VUserPrimaryGeneratorAction.hh"
export module GeantCore.Core.Interfaces.ISourceGenerator;
export namespace GeantCore::Core::Interfaces {
class ISourceGenerator : public G4VUserPrimaryGeneratorAction {
public:
  virtual ~ISourceGenerator() = default;

  virtual void GeneratePrimaries(G4Event *event) = 0;
};
} // namespace GeantCore::Core::Interfaces