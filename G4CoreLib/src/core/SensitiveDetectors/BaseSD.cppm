//
// Created by bunny on 30.03.26.
//
module;
#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>
#include <G4SystemOfUnits.hh> // ВАЖНО: Добавлено для единиц измерения (nm, um, mm)
#include <atomic>
#include <mutex>
#include <vector>
#include <algorithm>
#include <cmath> // Для std::floor и std::ceil

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
               const G4double ZBinWidth,      // Шаг сетки (например, 1.0 * nm)
               const G4double TotalThickness, // Полная толщина стопки слоев
               std::vector<LayerInfo> &globalProfile,
               std::mutex &profileMutex)
            : G4VSensitiveDetector(name),
              absorbedCount(abs),
              reflectedCount(ref),
              stackTop(StackTop),
              zBinWidth(ZBinWidth),
              fGlobalInfo(globalProfile),
              fMutex(profileMutex) {

            // 1. Вычисляем точное количество ячеек (бинов) в нашем векторе
            totalBins = static_cast<size_t>(std::ceil(TotalThickness / zBinWidth));

            // 2. Сразу выделяем память! Никаких динамических аллокаций во время симуляции
            localProfile.resize(totalBins);
        };

        ~BaseSD() override = default;

        BaseSD(const BaseSD &) = delete;
        BaseSD &operator=(const BaseSD &) = delete;
        BaseSD(const BaseSD &&) = delete;
        BaseSD &operator=(const BaseSD &&) = delete;

#pragma endregion

#pragma region Methods

    public:
        void Initialize(G4HCofThisEvent * /*hce*/) override {
            // Очень быстрое обнуление локального профиля перед новым событием
            // Мы не очищаем имена слоев (они статичны для геометрии), только сбрасываем энергию
            for (auto &bin : localProfile) {
                bin.Edep = 0.0f;
            }
        }

        bool ProcessHits(G4Step *step, G4TouchableHistory * /*history*/) override {
            CollectPrimaryInteractionInfo(step);
            CalculateEdepVsZ(step);
            return true;
        }

        void EndOfEvent(G4HCofThisEvent * /*hce*/) override {
            std::lock_guard<std::mutex> lock(fMutex);

            // Если глобальный вектор еще пуст, выделяем ему память один раз
            if (fGlobalInfo.empty()) {
                fGlobalInfo.resize(totalBins);
            }

            // Сливаем локальный массив с глобальным (элемент к элементу) - O(N)
            for (size_t i = 0; i < totalBins; ++i) {
                if (localProfile[i].Edep > 0.0f) {
                    fGlobalInfo[i].Edep += localProfile[i].Edep;

                    // Если в глобальном бине еще нет метаданных, копируем их
                    if (fGlobalInfo[i].layerName.empty()) {
                        fGlobalInfo[i].layerID = localProfile[i].layerID;
                        fGlobalInfo[i].layerName = localProfile[i].layerName;
                        fGlobalInfo[i].z_depth = localProfile[i].z_depth;
                        fGlobalInfo[i].EHP_count = 0.0f;
                    }
                }
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

            if (!touchable->GetVolume()) return;

            // 1. Считаем абсолютную глубину (сверху вниз)
            const G4double zMid = 0.5 * (pre->GetPosition().z() + post->GetPosition().z());
            const G4double depth = stackTop - zMid;

            // Защита: электрон может отразиться и улететь в "отрицательную" глубину (выше StackTop)
            if (depth < 0.0) return;

            // 2. Идеальный расчет индекса корзины с защитой от float-погрешностей
            size_t binIndex = static_cast<size_t>(std::floor(depth / zBinWidth));

            // 3. Строгая защита от выхода за пределы массива
            if (binIndex < totalBins) {

                // Накапливаем энергию в корзину! Быстрый доступ O(1)
                localProfile[binIndex].Edep += static_cast<float>(edep);

                // Если это первое попадание в этот бин за текущее событие, записываем данные слоя
                if (localProfile[binIndex].layerName.empty()) {
                    localProfile[binIndex].layerID = static_cast<uint8_t>(touchable->GetCopyNumber());
                    localProfile[binIndex].layerName = touchable->GetVolume()->GetName();

                    // === ИДЕАЛЬНОЕ ПРЕОБРАЗОВАНИЕ В НАНОМЕТРЫ ===
                    // Деление на CLHEP::nm гарантирует красивые и ровные числа (0.5, 1.5, 2.5...)
                    localProfile[binIndex].z_depth = static_cast<float>(((binIndex + 0.5) * zBinWidth) / CLHEP::nm);
                }
            }
        }

#pragma endregion

#pragma region Fields

    private:
        std::atomic<unsigned long long> &absorbedCount;
        std::atomic<unsigned long long> &reflectedCount;

        const G4double stackTop;
        const G4double zBinWidth;

        // Вектор вместо мапы
        size_t totalBins;
        std::vector<LayerInfo> localProfile;

        std::vector<LayerInfo> &fGlobalInfo;
        std::mutex &fMutex;

#pragma endregion
    };
}