module;
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
export module GeantCore.Core.Interfaces.IMaterials;

export namespace GeantCore::Core::Interfaces {
    class IMaterials
    {
        public:
        virtual ~IMaterials() = default;

        virtual std::optional<G4Material*> Get(const G4String &name);

        //virtual G4Material *BuildFromSpec(const std::string &name, const MaterialBuildSpec &spec);
    };
}