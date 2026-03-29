//
// Created by bunny on 28.03.26.
//
module;
#include <G4VUserPrimaryGeneratorAction.hh>
#include <memory>
export module GeantCore.Core.SourceGenerators.SourceGeneratorWrapper;
import GeantCore.Core.Concepts.SourceGeneratorConcept;
export namespace GeantCore::Core::SourceGenerators {
    using namespace GeantCore::Core::Concepts;
    class SourceGeneratorWrapper : public G4VUserPrimaryGeneratorAction {
    public:
        std::unique_ptr<G4VUserPrimaryGeneratorAction> currentSourceGenerator;

    public:
        template<SourceGeneratorConcept Detector>
        void SetSourceGenerator(std::unique_ptr<Detector> sourceGenerator) {
            currentSourceGenerator = std::move(sourceGenerator);
        }

        void GeneratePrimaries(G4Event *event) override {
            if (!currentSourceGenerator) return;

            currentSourceGenerator->GeneratePrimaries(event);
        }
    };
}
