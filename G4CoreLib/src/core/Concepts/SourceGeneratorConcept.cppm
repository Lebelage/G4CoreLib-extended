module;
#include <G4VUserPrimaryGeneratorAction.hh>
#include <concepts>
#include <G4Event.hh>
export module GeantCore.Core.Concepts.SourceGeneratorConcept;

export namespace GeantCore::Core::Concepts {
    template <typename T>
    concept SourceGeneratorConcept =
        std::derived_from<T, G4VUserPrimaryGeneratorAction> && requires(T source, G4Event* event) {
        { source.GeneratePrimaries(event) } -> std::same_as<void>;
        };
} // namespace GeantCore::Core::Concepts
