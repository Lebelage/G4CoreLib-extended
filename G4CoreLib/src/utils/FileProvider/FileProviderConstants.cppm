//
// Created by sonora on 30.03.2026.
//
module;
#include <string>
export module GeantCore.Utils.FileProviderConstants;
export namespace GeantCore::Utils::FileProvider {
    class FileProviderConstants {
    public:
        // Directories
        const static inline std::string __EXPERIMENTS_DIR_NAME = "Experiments";
        const static inline std::string __ANALYSIS_DIR_NAME = "Analysis";
        const static inline std::string __APPCONFIGS_DIR_NAME = "AppConfigs";

        // Files
        const static inline std::string __GAN_INGAN_EXP_FILE_NAME = "GaN_InGaN_exp.json";
    };

}
