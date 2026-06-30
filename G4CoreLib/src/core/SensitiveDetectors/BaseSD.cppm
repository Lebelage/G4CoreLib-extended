module;
#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>
#include <G4SystemOfUnits.hh>
#include <G4TouchableHistory.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4ios.hh>
#include <G4Electron.hh> // ДОБАВЛЕНО для фильтрации электронов

#include <atomic>
#include <mutex>
#include <unordered_set>
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
            std::vector<unsigned long long> &globalSpectrum, // ДОБАВЛЕНО
            const G4double maxSpectrumE,                     // ДОБАВЛЕНО
            const G4double spectrumBinWidth,                 // ДОБАВЛЕНО
            std::mutex &profileMutex
        )
            : G4VSensitiveDetector(name),
              absorbedCount(abs),
              reflectedCount(ref),
              eventCount(events),
              detectorThickness(DetectorThickness),
              zBinWidth(ZBinWidth),
              fGlobalInfo(globalProfile),
              fGlobalSpectrum(globalSpectrum),           // Инициализация
              fSpectrumBinWidth(spectrumBinWidth),       // Инициализация
              fMutex(profileMutex) {
            
            totalBins = static_cast<size_t>(std::ceil(detectorThickness / zBinWidth));
            if (totalBins == 0) totalBins = 1;
            localProfile.resize(totalBins);

            // Инициализация бинов спектра
            spectrumTotalBins = static_cast<size_t>(std::ceil(maxSpectrumE / spectrumBinWidth));
            if (spectrumTotalBins == 0) spectrumTotalBins = 1;
            localSpectrum.resize(spectrumTotalBins, 0);
        }

        ~BaseSD() override = default;

    public:
        void Initialize(G4HCofThisEvent * /*hce*/) override {
            for (auto &bin: localProfile) {
                bin.Edep = 0.0f;
                bin.EHP_count = 0.0f;
            }
            std::fill(localSpectrum.begin(), localSpectrum.end(), 0);
            processedPrimaries.clear();
        }

        bool ProcessHits(G4Step *step, G4TouchableHistory * /*history*/) override {
            if (!step) return false;
            CollectPrimaryInteractionInfo(step);
            CalculateEdepVsZ(step);
            CollectEnergySpectrum(step); // ВЫЗОВ НОВОГО МЕТОДА
            return true;
        }

        void EndOfEvent(G4HCofThisEvent * /*hce*/) override {
            eventCount.fetch_add(1, std::memory_order_relaxed);
            std::lock_guard<std::mutex> lock(fMutex);

            if (fGlobalInfo.empty()) fGlobalInfo.resize(totalBins);
            for (size_t i = 0; i < totalBins; ++i) {
                if (localProfile[i].Edep <= 0.0f) continue;
                fGlobalInfo[i].Edep += localProfile[i].Edep;
                if (fGlobalInfo[i].layerName.empty()) {
                    fGlobalInfo[i].layerID = localProfile[i].layerID;
                    fGlobalInfo[i].layerName = localProfile[i].layerName;
                    fGlobalInfo[i].z_depth = localProfile[i].z_depth;
                    fGlobalInfo[i].EHP_count = 0.0f;
                }
            }

            // Слияние спектра
            if (fGlobalSpectrum.empty()) fGlobalSpectrum.resize(spectrumTotalBins, 0);
            for (size_t i = 0; i < spectrumTotalBins; ++i) {
                fGlobalSpectrum[i] += localSpectrum[i];
            }
        }

    private:
        void CollectEnergySpectrum(const G4Step *step) {
            auto *track = step->GetTrack();
            if (!track) return;

            // 1. Ловим ТОЛЬКО электроны
            if (track->GetDefinition() != G4Electron::Electron()) return;

            auto *prePoint = step->GetPreStepPoint();

            // 2. Частица должна пересечь границу объема (только что вошла)
            if (prePoint->GetStepStatus() != fGeomBoundary) return;

            // 3. Защита от обратного рассеяния (Backscattering)
            // Электрон мог пролететь детектор насквозь, отразиться от молекулы воздуха
            // позади детектора и влететь в структуру СЗАДИ. Нам ведь нужен падающий спектр?
            // Предполагается, что источник бьет вдоль оси Z (от минуса к плюсу).
            if (prePoint->GetMomentumDirection().z() <= 0.0) {
                return; // Игнорируем частицы, влетающие с обратной стороны
            }

            // 4. Отсекаем "мусорные" вторичные электроны (дельта-электроны).
            // Если вы стреляете из ParticleGun/GPS, первичный электрон имеет ParentID == 0.
            // Если используете G4RadioactiveDecay (распад Ni63), бета-электрон имеет ParentID > 0.
            // Чтобы не усложнять с ID, мы просто проверяем, что электрон родился НЕ в детекторе.
            if (track->GetLogicalVolumeAtVertex()->GetName() == "LayerLV") {
                return;
            }

            G4int trackID = track->GetTrackID();

            // 5. Защита от двойного счета (если детектор состоит из нескольких слоев LayerLV)
            if (processedPrimaries.find(trackID) != processedPrimaries.end()) return;

            G4double kineticEnergy = prePoint->GetKineticEnergy();

            size_t bin = static_cast<size_t>(std::floor(kineticEnergy / fSpectrumBinWidth));
            if (bin >= spectrumTotalBins) bin = spectrumTotalBins - 1;

            localSpectrum[bin]++;

            processedPrimaries.insert(trackID);
        }
    
        void CollectPrimaryInteractionInfo(const G4Step *step) {
            auto *track = step->GetTrack();
            if (!track) return;
            // Учитываем электроны для отражения
            if (track->GetDefinition() != G4Electron::Electron()) return;

            auto *postPoint = step->GetPostStepPoint();
            if (!postPoint) return;
            if (postPoint->GetStepStatus() != fGeomBoundary) return;

            auto touchable = postPoint->GetTouchableHandle();
            if (!touchable || !touchable->GetVolume()) return;
            auto *postLV = touchable->GetVolume()->GetLogicalVolume();
            if (!postLV) return;

            if (postLV->GetName() != "LayerLV") {
                if (postPoint->GetMomentumDirection().z() > 0.0) {
                    reflectedCount.fetch_add(1, std::memory_order_relaxed);
                    track->SetTrackStatus(fStopAndKill);
                }
            }
        }

        void CalculateEdepVsZ(const G4Step *step) {
            // Ваш код CalculateEdepVsZ остается абсолютно без изменений
            const G4double edep = step->GetTotalEnergyDeposit();
            if (edep <= 0.0) return;
            const auto *prePoint = step->GetPreStepPoint();
            const auto *postPoint = step->GetPostStepPoint();
            if (!prePoint || !postPoint) return;
            const G4double zPre = prePoint->GetPosition().z();
            const G4double zPost = postPoint->GetPosition().z();
            const G4double zMid = 0.5 * (zPre + zPost);
            const G4double depth = -zMid;
            if (depth < 0.0 || depth >= detectorThickness) return;
            size_t binIndex = static_cast<size_t>(std::floor(depth / zBinWidth));
            if (binIndex >= totalBins) return;
            localProfile[binIndex].Edep += static_cast<float>(edep);

            if (localProfile[binIndex].layerName.empty()) {
                
                auto touchable = prePoint->GetTouchableHandle();
                if (touchable && touchable->GetVolume()) {
                    localProfile[binIndex].layerID = static_cast<uint8_t>(touchable->GetCopyNumber());
                    localProfile[binIndex].layerName = touchable->GetVolume()->GetName();
                } else {
                    localProfile[binIndex].layerID = 0;
                    localProfile[binIndex].layerName = "Unknown";
                }

                localProfile[binIndex].z_depth = static_cast<float>(((static_cast<G4double>(binIndex) + 0.5) * zBinWidth) / nm);
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

        std::vector<unsigned long long> &fGlobalSpectrum;
        const G4double fSpectrumBinWidth;
        size_t spectrumTotalBins = 0;
        std::vector<unsigned long long> localSpectrum;
        std::unordered_set<G4int> processedPrimaries;

        std::mutex &fMutex;
    };
}