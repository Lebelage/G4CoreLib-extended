module;
#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
export module GeantCore.Core.Interfaces.IDetectorConstruction;

export namespace GeantCore::Core::Interfaces
{   
    class IDetectorConstruction : public G4VUserDetectorConstruction
    {
        public:
        virtual ~IDetectorConstruction() = default;

        virtual G4VPhysicalVolume* BuildWorld() const = 0;

        virtual G4double GetTotalThickness() const = 0;
        virtual G4double GetStackTopZ() const = 0; 
    };
}