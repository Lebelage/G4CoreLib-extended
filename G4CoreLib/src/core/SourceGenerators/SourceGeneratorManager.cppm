//
// Created by bunny on 28.03.26.
//
module;
#include <G4VUserPrimaryGeneratorAction.hh>
#include <memory>
export module GeantCore.Core.SourceGenertors.SourceGeneratorManager;
import GeantCore.Core.SourceGenerators.SourceGeneratorWrapper;
import GeantCore.Core.Concepts.SourceGeneratorConcept;
export namespace GeantCore::Core::SourceGenerators {
    class SourceGeneratorManager {
    public:
        SourceGeneratorManager() {
            sourceWrapper = std::make_unique<SourceGeneratorWrapper>();
        };

        ~SourceGeneratorManager() {
        };

        SourceGeneratorManager(const SourceGeneratorManager &) = delete;

        SourceGeneratorManager &operator=(const SourceGeneratorManager &) = delete;

        SourceGeneratorManager(const SourceGeneratorManager &&) = delete;

        SourceGeneratorManager &operator=(const SourceGeneratorManager &&) = delete;

    public:
        template<SourceGeneratorConcept SourceGenerator>
        void SetSourceGenerator(std::unique_ptr<SourceGenerator> source) {
            sourceWrapper->SetSourceGenerator(std::move(source));
        }

        G4VUserPrimaryGeneratorAction *GetCurrentSourceGeneratorPointer() {
            return sourceWrapper.get();
        }

    private:
        std::unique_ptr<SourceGeneratorWrapper> sourceWrapper;
    };
}
