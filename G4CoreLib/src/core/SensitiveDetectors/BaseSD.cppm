//
// Created by bunny on 30.03.26.
//
module;
#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>
#include <atomic>
export module GeantCore.Core.SensitiveDetectors.BaseSD;
export namespace GeantCore::Core::SensitiveDetectors {
    class BaseSD : public G4VSensitiveDetector {
#pragma region Constructors/Destructor

    public:
        BaseSD(G4String name,
               std::atomic<unsigned long long> &abs,
               std::atomic<unsigned long long> &ref)
            : G4VSensitiveDetector(name), absorbedCount(abs), reflectedCount(ref) {
        };

        ~BaseSD() override {
        };

        BaseSD(const BaseSD &) = delete;

        BaseSD &
        operator=(const BaseSD &) = delete;

        BaseSD(const BaseSD &&) = delete;

        BaseSD &
        operator=(const BaseSD &&) = delete;
#pragma endregion

#pragma region Methods
        bool ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) override {
            auto* track = step->GetTrack();

            if (track->GetParentID() != 0) return false;

            if (track->GetTrackStatus() == fStopAndKill) {
                absorbedCount.fetch_add(1, std::memory_order_relaxed);
                return true;
            }

            auto* postPoint = step->GetPostStepPoint();

            if (postPoint->GetStepStatus() == fGeomBoundary) {

                auto* postVol = postPoint->GetTouchableHandle()->GetVolume();

                if (!postVol || postVol->GetLogicalVolume()->GetName() != "LayerLV") {

                    if (postPoint->GetMomentumDirection().z() > 0) {
                        reflectedCount.fetch_add(1, std::memory_order_relaxed);
                        track->SetTrackStatus(fStopAndKill);

                        return true;
                    }
                }
            }

            return false;
        }
#pragma endregion

#pragma region Fields

    private:
        std::atomic<unsigned long long> &absorbedCount;
        std::atomic<unsigned long long> &reflectedCount;
#pragma endregion
    };
}
