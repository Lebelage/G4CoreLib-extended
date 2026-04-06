//
// Created by bunny on 30.03.26.
//
module;
#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>
#include <atomic>
#include <G4AnalysisManager.hh>
#include <G4RunManager.hh>
#include <queue>

#include "G4Types.hh"
export module GeantCore.Core.SensitiveDetectors.BaseSD;
import GeantCore.Core.PostProcessManager;
export namespace GeantCore::Core::SensitiveDetectors {
    using namespace GeantCore::Core;
    class BaseSD : public G4VSensitiveDetector {
#pragma region Constructors/Destructor

    public:
        BaseSD(G4String name,
               std::atomic<unsigned long long> &abs,
               std::atomic<unsigned long long> &ref,
               const G4double StackTop,
               const G4double ZBinWidth,
               std::vector<LayerInfo>& globalProfile, // Ссылка на глобальную мапу
               std::mutex& profileMutex)                 // Ссылка на глобальный мьютекс
            : G4VSensitiveDetector(name),
              absorbedCount(abs),
              reflectedCount(ref),
              stackTop(StackTop),
              zBinWidth(ZBinWidth),
              fGlobalInfo(globalProfile),
              fMutex(profileMutex) {
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
        void Initialize(G4HCofThisEvent* /*hce*/) override {
            // Очищаем ЛОКАЛЬНУЮ мапу перед началом полета новой частицы
            localInfo = {};
        }

        bool ProcessHits(G4Step *step, G4TouchableHistory * /*history*/) override {
            CollectPrimaryInteractionInfo(step);
            CalculateEdepVsZ(step);
            return true;
        }

        void EndOfEvent(G4HCofThisEvent* /*hce*/) override {
            std::lock_guard<std::mutex> lock(fMutex);

            while (!localInfo.empty()) {
                fGlobalInfo.push_back(std::move(localInfo.front()));
                localInfo.pop();
            }
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
            if (edep <= 0.) return;

            auto pre = step->GetPreStepPoint();
            auto post = step->GetPostStepPoint();
            auto touchable = pre->GetTouchableHandle();

            if (!touchable->GetVolume() || !post->GetTouchableHandle()->GetVolume())
                return;

            const G4double zMid = 0.5 * (pre->GetPosition().z() + post->GetPosition().z());
            const G4double depth = stackTop - zMid;

            LayerInfo info{};
            info.layerID = static_cast<uint8_t>(touchable->GetCopyNumber());
            info.layerName = touchable->GetVolume()->GetName();
            info.z_depth = static_cast<float>(depth);
            info.Edep = static_cast<float>(edep);

            localInfo.emplace(info);
        }


#pragma endregion

#pragma region Fields

    private:
        std::atomic<unsigned long long> &absorbedCount;
        std::atomic<unsigned long long> &reflectedCount;
        const G4double stackTop;
        const G4double zBinWidth;

        std::queue<LayerInfo> localInfo;

        std::vector<LayerInfo>& fGlobalInfo;
        std::mutex& fMutex;

#pragma endregion
    };
}
