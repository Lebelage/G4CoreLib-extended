//
// Created by bunny on 30.03.26.
//
module;

#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>
#include <G4SystemOfUnits.hh>
#include <G4TouchableHistory.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4ios.hh>

#include <atomic>
#include <mutex>
#include <vector>
#include <cmath>

#include "G4Types.hh"

export module GeantCore.Core.SensitiveDetectors.BaseSD;

import GeantCore.Core.PostProcessManager;

export namespace GeantCore::Core::SensitiveDetectors {
    using namespace GeantCore::Core;

    class BaseSD : public G4VSensitiveDetector {
    public:
        BaseSD(
            G4String name,
            std::atomic<unsigned long long> &abs,
            std::atomic<unsigned long long> &ref,
            std::atomic<unsigned long long> &events,
            const G4double DetectorThickness,
            const G4double ZBinWidth,
            std::vector<LayerInfo> &globalProfile,
            std::mutex &profileMutex
        )
            : G4VSensitiveDetector(name),
              absorbedCount(abs),
              reflectedCount(ref),
              eventCount(events),
              detectorThickness(DetectorThickness),
              zBinWidth(ZBinWidth),
              fGlobalInfo(globalProfile),
              fMutex(profileMutex) {
            totalBins = static_cast<size_t>(
                std::ceil(detectorThickness / zBinWidth)
            );

            if (totalBins == 0) {
                totalBins = 1;
            }

            localProfile.resize(totalBins);
        }

        ~BaseSD() override = default;

        BaseSD(const BaseSD &) = delete;

        BaseSD &operator=(const BaseSD &) = delete;

        BaseSD(BaseSD &&) = delete;

        BaseSD &operator=(BaseSD &&) = delete;

    public:
        void Initialize(G4HCofThisEvent * /*hce*/) override {
            for (auto &bin: localProfile) {
                bin.Edep = 0.0f;
                bin.EHP_count = 0.0f;
            }
        }

        bool ProcessHits(G4Step *step, G4TouchableHistory * /*history*/) override {
            if (!step) {
                return false;
            }

            CollectPrimaryInteractionInfo(step);
            CalculateEdepVsZ(step);

            return true;
        }

        void EndOfEvent(G4HCofThisEvent * /*hce*/) override {
            eventCount.fetch_add(1, std::memory_order_relaxed);

            std::lock_guard<std::mutex> lock(fMutex);

            if (fGlobalInfo.empty()) {
                fGlobalInfo.resize(totalBins);
            }

            for (size_t i = 0; i < totalBins; ++i) {
                if (localProfile[i].Edep <= 0.0f) {
                    continue;
                }

                fGlobalInfo[i].Edep += localProfile[i].Edep;

                if (fGlobalInfo[i].layerName.empty()) {
                    fGlobalInfo[i].layerID = localProfile[i].layerID;
                    fGlobalInfo[i].layerName = localProfile[i].layerName;
                    fGlobalInfo[i].z_depth = localProfile[i].z_depth;
                    fGlobalInfo[i].EHP_count = 0.0f;
                }
            }
        }

    private:
        void CollectPrimaryInteractionInfo(const G4Step *step) {
            auto *track = step->GetTrack();

            if (!track) {
                return;
            }

            if (track->GetParentID() != 0) {
                return;
            }

            auto *postPoint = step->GetPostStepPoint();

            if (!postPoint) {
                return;
            }

            if (postPoint->GetStepStatus() != fGeomBoundary) {
                return;
            }

            auto touchable = postPoint->GetTouchableHandle();

            if (!touchable || !touchable->GetVolume()) {
                return;
            }

            auto *postVol = touchable->GetVolume();
            auto *postLV = postVol->GetLogicalVolume();

            if (!postLV) {
                return;
            }

            const auto &postLVName = postLV->GetName();

            // Частица вышла из детектора наружу
            if (postLVName != "LayerLV") {
                // Детектор находится в -Z.
                // Отражение — выход обратно вверх, к источнику, то есть в +Z.
                if (postPoint->GetMomentumDirection().z() > 0.0) {
                    reflectedCount.fetch_add(1, std::memory_order_relaxed);
                    track->SetTrackStatus(fStopAndKill);
                }
            }
        }

        void CalculateEdepVsZ(const G4Step *step) {
            const G4double edep = step->GetTotalEnergyDeposit();

            if (edep <= 0.0) {
                return;
            }

            const auto *prePoint = step->GetPreStepPoint();
            const auto *postPoint = step->GetPostStepPoint();

            if (!prePoint || !postPoint) {
                return;
            }

            const G4double zPre = prePoint->GetPosition().z();
            const G4double zPost = postPoint->GetPosition().z();

            const G4double zMid = 0.5 * (zPre + zPost);

            // Детектор: 0 -> -Z
            // depth = 0 на интерфейсе
            // depth > 0 внутри детектора
            const G4double depth = -zMid;

            if (depth < 0.0) {
                return;
            }

            if (depth >= detectorThickness) {
                return;
            }

            size_t binIndex = static_cast<size_t>(
                std::floor(depth / zBinWidth)
            );

            if (binIndex >= totalBins) {
                return;
            }

            localProfile[binIndex].Edep += static_cast<float>(edep);

            if (localProfile[binIndex].layerName.empty()) {
                auto touchable = prePoint->GetTouchableHandle();

                if (touchable && touchable->GetVolume()) {
                    localProfile[binIndex].layerID =
                            static_cast<uint8_t>(touchable->GetCopyNumber());

                    localProfile[binIndex].layerName =
                            touchable->GetVolume()->GetName();
                } else {
                    localProfile[binIndex].layerID = 0;
                    localProfile[binIndex].layerName = "Unknown";
                }

                localProfile[binIndex].z_depth =
                        static_cast<float>(
                            ((static_cast<G4double>(binIndex) + 0.5) * zBinWidth) / nm
                        );

                localProfile[binIndex].EHP_count = 0.0f;
            }
        }

    private:
        std::atomic<unsigned long long> &absorbedCount;
        std::atomic<unsigned long long> &reflectedCount;
        std::atomic<unsigned long long> &eventCount;

        const G4double detectorThickness;
        const G4double zBinWidth;

        size_t totalBins = 0;

        std::vector<LayerInfo> localProfile;

        std::vector<LayerInfo> &fGlobalInfo;
        std::mutex &fMutex;
    };
}
