//
// Created by bunny on 6.04.26.
//
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
#pragma region Constructors/destructor

    public:
        static PostProcessManager &getInstance() {
            static PostProcessManager instance;
            return instance;
        }

        PostProcessManager(const PostProcessManager &) = delete;

        PostProcessManager &operator=(const PostProcessManager &) = delete;

        PostProcessManager(PostProcessManager &&) = delete;

        PostProcessManager &operator=(PostProcessManager &&) = delete;

    private:
        PostProcessManager() = default;

        ~PostProcessManager() = default;
#pragma endregion
#pragma region Methods

    public:
        void PostProcess(std::vector<LayerInfo> &&layersInfo,
                         std::unordered_map<uint8_t, Materials::ExtendedG4Material> &&layersMapArg) {
            layers = std::move(layersInfo);
            layerMap = std::move(layersMapArg);

            // Исправлено: обращаемся к layerMap, а не к layersMapArg
            auto updatableLayers = layers | std::views::filter([&](const LayerInfo &layerInfo) {
                return layerMap.contains(layerInfo.layerID);
            });

            std::ranges::for_each(updatableLayers, [&](LayerInfo &l) {
                auto &extMat = layerMap.at(l.layerID);
                if (extMat.GetG4Material()) {
                    l.layerName = extMat.GetG4Material()->GetName();
                    CalculateEHP(l, extMat.GetEg());
                }
            });

        }

        std::string SerializeLayersToJson() const {
            // Благодаря функции to_json выше, вектор конвертируется в json-массив автоматически
            json j = layers;

            // Возвращаем строку с отступами в 4 пробела для красивого форматирования
            return j.dump(4);
        }

    private:
        void CalculateEHP(LayerInfo &layerInfo, float Eg) {
            float E_EHP_eV = 2.8f * Eg + 0.6f;

            if (E_EHP_eV > 0.0f) {
                layerInfo.EHP_count = (layerInfo.Edep * 1e6f) / E_EHP_eV;
            } else {
                layerInfo.EHP_count = 0.0f;
            }
        }
#pragma endregion

#pragma region Fields

    private:
        std::vector<LayerInfo> layers;
        std::unordered_map<uint8_t, Materials::ExtendedG4Material> layerMap;
#pragma endregion
    };
}
