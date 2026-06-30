module;
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4RunManager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4String.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "G4Types.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4MTRunManager.hh"
#include "G4VUserActionInitialization.hh"
#include "G4IonTable.hh"

export module GeantCore.Externals;
export namespace GeantModules
{
    auto a = 1 * um;
    using ::G4IonTable;
    using ::G4VUserActionInitialization;
    using ::G4MTRunManager;
    using ::G4RunManager;
    using ::G4UIExecutive;
    using ::G4UImanager;
    using ::G4VisExecutive;
    using ::FTFP_BERT;
    using ::G4EmStandardPhysics_option4;
    using ::G4StepLimiterPhysics;
    using ::G4DecayPhysics;
    using ::G4RadioactiveDecayPhysics;
    using ::G4String;
    using ::G4AnalysisManager;
    using ::G4UserRunAction;
    using ::G4Run;  
    using ::G4double;
    using ::G4int;

    inline constexpr G4double um  = CLHEP::um;
    inline constexpr G4double mm  = CLHEP::mm;
    inline constexpr G4double cm  = CLHEP::cm;
    inline constexpr G4double m   = CLHEP::m;

    inline constexpr G4double eV  = CLHEP::eV;
    inline constexpr G4double keV = CLHEP::keV;
    inline constexpr G4double MeV = CLHEP::MeV;

}