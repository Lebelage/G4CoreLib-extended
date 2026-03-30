cmake_minimum_required(VERSION 3.28)
project(G4CoreLib LANGUAGES CXX)

if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path
                    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT} CACHE PATH "" FORCE)

    add_compile_options("-I/opt/homebrew/opt/llvm/include")
    add_link_options("-L/opt/homebrew/opt/llvm/lib" "-Wl,-rpath,/opt/homebrew/opt/llvm/lib")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Geant4 REQUIRED)

# 1. Поиск всех интерфейсов модулей (.cppm)
file(GLOB_RECURSE ALL_MODULE_INTERFACES "src/*.cppm")

# 2. Поиск всех обычных файлов реализации (.cpp)
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")

add_library(GeantCoreLib STATIC
        src/core/Detectors/DetectorWrapper.cppm
        src/core/Detectors/StartDetector.cppm
        src/core/SourceGenerators/SourceGeneratorWrapper.cppm
        src/core/Concepts/SourceGeneratorConcept.cppm
        src/core/SourceGenerators/SourceGeneratorManager.cppm
        src/core/SensitiveDetectors/BaseSD.cppm)

# 3. Привязка файлов
target_sources(GeantCoreLib
    PUBLIC
        # Модули выделяем в отдельный сет для генерации интерфейсов
        FILE_SET CXX_MODULES FILES ${ALL_MODULE_INTERFACES}
    PRIVATE
        # Обычные cpp файлы просто компилируются
        ${ALL_SOURCES}
)

target_link_libraries(GeantCoreLib PUBLIC ${Geant4_LIBRARIES})
target_include_directories(GeantCoreLib PUBLIC ${Geant4_INCLUDE_DIRS})
target_compile_definitions(GeantCoreLib PUBLIC ${Geant4_DEFINITIONS})