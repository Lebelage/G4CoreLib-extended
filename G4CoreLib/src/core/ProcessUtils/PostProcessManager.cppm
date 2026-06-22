module;
#include <G4Material.hh>
#include <ranges>
#include <unordered_map>
#include <nlohmann/json.hpp>

export module GeantCore.Core.PostProcessManager;
import GeantCore.Core.Materials.ExtendedG4Material;
import GeantCore.Core.Materials.MaterialsConstants;

export namespace GeantCore::Core {
    using json = nlohmann::json;

    struct LayerInfo {
        uint8_t layerID;
        std::string layerName;
        float z_depth;
        float Edep;
        float EHP_count;
    };

    void to_json(json &j, const LayerInfo &l) {
        j = json{
            {"layerID", l.layerID},
            {"layerName", l.layerName},
            {"z_depth", l.z_depth},
            {"Edep_MeV", l.Edep},
            {"EHP_count", l.EHP_count}
        };
    }

    class PostProcessManager {
    public:
        static PostProcessManager &getInstance() {
            static PostProcessManager instance;
            return instance;
        }

        PostProcessManager(const PostProcessManager &) = delete;
        PostProcessManager &operator=(const PostProcessManager &) = delete;

    private:
        PostProcessManager() = default;
        ~PostProcessManager() = default;

    public:
        void PostProcess(std::vector<LayerInfo> &&layersInfo,
                         std::vector<unsigned long long> &&spectrumInfo,
                         double binWidth, // ДОБАВЛЕНО: Ширина бина энергии
                         std::unordered_map<uint8_t, Materials::ExtendedG4Material> &&layersMapArg,
                         unsigned long long totalEvents) {
            layers = std::move(layersInfo);
            spectrum = std::move(spectrumInfo);
            spectrumBinWidth = binWidth; // Сохраняем ширину бина
            layerMap = std::move(layersMapArg);

            auto updatableLayers = layers | std::views::filter([&](const LayerInfo &layerInfo) {
                return layerMap.contains(layerInfo.layerID);
            });

            std::ranges::for_each(updatableLayers, [&](LayerInfo &l) {
                auto &extMat = layerMap.at(l.layerID);
                if (extMat.GetG4Material()) {
                    l.layerName = extMat.GetG4Material()->GetName();

                    if (totalEvents > 0) {
                        l.Edep /= static_cast<float>(totalEvents);
                    }

                    CalculateEHP(l, extMat.GetEg());
                }
            });
        }

        std::string SerializeLayersToJson() const {
            json j = layers;
            return j.dump(4);
        }

        // ИЗМЕНЕНО: Формируем пары {x: энергия, y: количество}
        std::string SerializeSpectrumToJson() const {
            json j = json::array();
            for (size_t i = 0; i < spectrum.size(); ++i) {
                j.push_back({
                    {"x", static_cast<double>(i) * spectrumBinWidth}, // Энергия
                    {"y", spectrum[i]}                                // Количество электронов
                });
            }
            return j.dump(4);
        }

    private:
        void CalculateEHP(LayerInfo &layerInfo, float Eg) {
            float E_EHP_eV = 2.8f * Eg + 0.6f;

            if (layerInfo.layerName == "G4_Au") return;
            if (layerInfo.layerName == "G4_Ti") return;

            if (E_EHP_eV > 0.0f) {
                layerInfo.EHP_count = (layerInfo.Edep * 1e6f) / E_EHP_eV;
            } else {
                layerInfo.EHP_count = 0.0f;
            }
        }

    private:
        std::vector<LayerInfo> layers;
        std::vector<unsigned long long> spectrum;
        double spectrumBinWidth = 1.0; // Значение по умолчанию
        std::unordered_map<uint8_t, Materials::ExtendedG4Material> layerMap;
    };
}