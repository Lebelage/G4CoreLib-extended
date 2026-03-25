module;
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserRunAction.hh"
export module GeantCore.Core.Actions.BaseRunAction;
import GeantCore.Core.Interfaces.IRunAction;
export namespace GeantCore::Core::Actions {
using namespace GeantCore::Core::Interfaces;

class BaseRunAction : public IRunAction {
#pragma region Constructor/Destructor
public:
  BaseRunAction() { Initialize(); };
  ~BaseRunAction() { Release(); };

  BaseRunAction(const BaseRunAction &) = delete;
  BaseRunAction &operator=(const BaseRunAction &) = delete;

#pragma endregion

#pragma region Methods
  void BeginOfRunAction(const G4Run *) override {
    auto *ana = G4AnalysisManager::Instance();
    ana->OpenFile("out");
  }
  void EndOfRunAction(const G4Run *) override {
    auto *ana = G4AnalysisManager::Instance();
    ana->Write();
    ana->CloseFile();
  }

private:
  void Initialize() {
    auto *ana = G4AnalysisManager::Instance();
    ana->SetDefaultFileType("csv");
    ana->SetNtupleMerging(true);

    // depth: 0..3.1 um
    const G4int nbins = 3100;
    const G4double dmin = 0.0;
    const G4double dmax = 3.1 * um;
    ana->CreateH1("Edep_vs_depth", "Edep vs depth; depth [um]; Edep [MeV]",
                  nbins, dmin, dmax);

    // ana->CreateH1("Edep_step", "Energy deposit per step; dE (MeV); counts",
    //           400, 0.0, 0.01*MeV);

    ana->CreateNtuple("steps", "Step edep in layers");
    ana->CreateNtupleIColumn("eventID"); // 0
    ana->CreateNtupleIColumn("trackID"); // 1
    ana->CreateNtupleIColumn("stepNo");  // 2
    ana->CreateNtupleIColumn("layer");   // 3 (copyNo)
    ana->CreateNtupleDColumn("depth");   // 4
    ana->CreateNtupleDColumn("edep");    // 5
    ana->FinishNtuple();
  }
  void Release() { delete G4AnalysisManager::Instance(); }
#pragma endregion
};
} // namespace GeantCore::Core::Actions