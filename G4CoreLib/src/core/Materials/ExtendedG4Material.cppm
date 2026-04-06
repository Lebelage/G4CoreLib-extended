//
// Created by bunny on 6.04.26.
//
module;
#include <G4Material.hh>
export module GeantCore.Core.Materials.ExtendedG4Material;
export namespace GeantCore::Core::Materials {
    class ExtendedG4Material {
    public:
        ExtendedG4Material(G4Material *material, float x, float Eg, bool isAlloy)
            : mat{material}, x{x}, Eg{Eg}, isAlloy{isAlloy} {
        };

        ~ExtendedG4Material() {
        };

    public:
        float GetX() { return x; }
        float GetEg() { return Eg; }
        G4Material *GetG4Material() { return mat; }
        bool IsAlloy() { return isAlloy; }

    private:
        G4Material *mat;
        float x;
        float Eg;
        bool isAlloy;
    };
}
