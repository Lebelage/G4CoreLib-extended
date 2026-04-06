//
// Created by bunny on 6.04.26.
//
module;
#include <G4Material.hh>

export module GeantCore.Core.PostProcessManager;
import GeantCore.Core.Materials.ExtendedG4Material;
export namespace GeantCore::Core {

    struct LayerInfo {
        Materials::ExtendedG4Material* material;
        uint8_t layerID;
        std::string layerName;
        float totalEdep;
    };

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
        PostProcessManager();

        ~PostProcessManager();
#pragma endregion
#pragma region Methods
        void PostProcess() {
        }

        void SetEdepToLayer(uint8_t layerID, float Edep)
        {
            for (auto& l : layers) {
                if (l.layerID == layerID)
                    l.totalEdep = Edep;
            }
        }

        void CalculateEHP(LayerInfo *layerInfo) {

        }
#pragma endregion

#pragma region Fields




    private:
        std::vector<LayerInfo> layers;
#pragma endregion
    };
}
