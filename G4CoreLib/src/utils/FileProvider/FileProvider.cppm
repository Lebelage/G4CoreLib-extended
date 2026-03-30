//
// Created by sonora on 30.03.2026.
//
module;
#include <exception>
#include <filesystem>
#include <fstream>
export module GeantCore.Utils.FileProvider;
import GeantCore.Utils.FileProviderConstants;
namespace fs = std::filesystem;
export namespace GeantCore::Utils::FileProvider {
    class FileProvider {
    public:
        static bool CreateDirectory(const std::string &name) {
            try {
                fs::path directory = name;

                if (fs::create_directory(directory))
                    return true;

                throw std::runtime_error("Cannot create directory: " + name);
            } catch (std::exception ex) {
                return false;
            }
        }

        static bool CreateFile(const std::string &filePath) {
            try {
                if (fs::exists(filePath))
                    throw std::runtime_error("File existing: " + filePath);

                std::ofstream file(filePath);
                return file.good();
            } catch (std::exception ex) {
                return false;
            }
        }

        static bool CreateAndWriteToExperimentFile(const std::string &fileName, std::string && info) {
            try {
                fs::path dirPath = FileProviderConstants::__EXPERIMENTS_DIR_NAME;
                fs::path filePath = dirPath / fileName;

                if (!fs::exists(dirPath)) {
                    fs::create_directories(dirPath);
                }

                if (fs::exists(filePath)) {
                    return false;
                }

                std::ofstream file(filePath);

                if (file.good()) {
                    file << info;
                    file.close();
                }

                return file.good();
            } catch (const std::exception &ex) {
                return false;
            }
        }
    };
}
