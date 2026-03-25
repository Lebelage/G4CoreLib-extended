module;
#include "G4VUserDetectorConstruction.hh"
#include <concepts>
export module GeantCore.Core.Concepts.DetectorConstructionConcept;

export namespace GeantCore::Core::Concepts {
template <typename T>
concept DetectorConstructionConcept =
    std::derived_from<T, G4VUserDetectorConstruction> && requires(T det) {
      { det.Construct() } -> std::same_as<G4VPhysicalVolume *>;
    };
} // namespace GeantCore::Core::Concepts
