//
// Created by bunny on 30.03.26.
//
module;
#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>
#include <atomic>
#include <G4AnalysisManager.hh>
#include <G4RunManager.hh>
#include "G4Types.hh"
export module GeantCore.Core.SensitiveDetectors.BaseSD;
export namespace GeantCore::Core::SensitiveDetectors {
    class BaseSD : public G4VSensitiveDetector {
#pragma region Constructors/Destructor

    public:
        BaseSD(G4String name,
               std::atomic<unsigned long long> &abs,
               std::atomic<unsigned long long> &ref,
               const G4double StackTop)
            : G4VSensitiveDetector(name), absorbedCount(abs), reflectedCount(ref), stackTop(StackTop) {
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

    public:
        bool ProcessHits(G4Step *step, G4TouchableHistory * /*history*/) override {
            CollectPrimaryInteractionInfo(step);
            CalculateEdepVsZ(step);
            return true;
        }

    private:
        void CollectPrimaryInteractionInfo(const G4Step *step) {
            auto *track = step->GetTrack();

            if (track->GetParentID() != 0) return;

            if (track->GetTrackStatus() == fStopAndKill) {
                absorbedCount.fetch_add(1, std::memory_order_relaxed);
                return;
            }

            auto *postPoint = step->GetPostStepPoint();

            if (postPoint->GetStepStatus() == fGeomBoundary) {
                auto *postVol = postPoint->GetTouchableHandle()->GetVolume();

                if (!postVol || postVol->GetLogicalVolume()->GetName() != "LayerLV") {
                    if (postPoint->GetMomentumDirection().z() > 0) {
                        reflectedCount.fetch_add(1, std::memory_order_relaxed);
                        track->SetTrackStatus(fStopAndKill);
                    }
                }
            }
        }

        void CalculateEdepVsZ(const G4Step *step) {

            const G4double edep = step->GetTotalEnergyDeposit();
            if (edep <= 0.)
                return;

            auto dx = step->GetStepLength();

            auto pre = step->GetPreStepPoint();
            auto post = step->GetPostStepPoint();

            auto preVol = pre->GetTouchableHandle()->GetVolume();
            auto postVol = post->GetTouchableHandle()->GetVolume();

            if (!preVol || !postVol)
                return;

            // координата шага
            const G4double zMid =
                0.5 * (pre->GetPosition().z() + post->GetPosition().z());

            // глубина внутрь стека (0 на верхней поверхности, дальше по лучу)
            const G4double depth = stackTop - zMid;

            // if (depth < 0.0 || depth > fDet->GetTotalThickness())
            //     return;

            // IDs
            const auto *evt = G4RunManager::GetRunManager()->GetCurrentEvent();
            const G4int eventID = evt ? evt->GetEventID() : -1;

            const auto *tr = step->GetTrack();
            const G4int trackID = tr->GetTrackID();
            const G4int stepNo = tr->GetCurrentStepNumber();

            const G4int copyNo = pre->GetTouchableHandle()->GetCopyNumber();

            auto *ana = G4AnalysisManager::Instance();

            // H1(0): E(depth)

            ana->FillH1(0, depth, edep);

            // if (dx > 0 && edep > 0) ana->FillH1(1, edep/dx);

            // if (edep > 0) ana->FillH1(1, edep);

            // ntuple: edep за шаг
            ana->FillNtupleIColumn(0, eventID);
            ana->FillNtupleIColumn(1, trackID);
            ana->FillNtupleIColumn(2, stepNo);
            ana->FillNtupleIColumn(3, copyNo);
            ana->FillNtupleDColumn(4, depth);
            ana->FillNtupleDColumn(5, edep);
            ana->AddNtupleRow();
        }
#pragma endregion

#pragma region Fields

    private:
        std::atomic<unsigned long long> &absorbedCount;
        std::atomic<unsigned long long> &reflectedCount;
        const G4double stackTop;

#pragma endregion
    };
}
