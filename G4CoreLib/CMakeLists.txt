cmake_minimum_required(VERSION 4.3.3) # Рекомендуется 3.28+ для полноценной нативной поддержки C++ модулей
project(G4CoreLib LANGUAGES CXX)

# --- Настройки для macOS (LLVM Clang из Homebrew) ---
if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path
                    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT} CACHE PATH "" FORCE)

    # Гарантируем, что CMake использует кастомный LLVM toolchain
    add_compile_options("-I/opt/homebrew/opt/llvm/include")
    add_link_options("-L/opt/homebrew/opt/llvm/lib" "-Wl,-rpath,/opt/homebrew/opt/llvm/lib")
endif()

# --- Стандарт C++23 ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Поиск внешних зависимостей ---
find_package(Geant4 REQUIRED)

# --- Подгрузка сторонних библиотек (JSON) через релизный архив ---
include(FetchContent)

# Отключаем сборку тестов и документации самой библиотеки JSON
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# --- Поиск исходных файлов проекта ---

# 1. Находим абсолютно все интерфейсы модулей (.cppm) в проекте
file(GLOB_RECURSE ALL_MODULE_INTERFACES 
    "src/*.cppm"
    "externals/*.cppm"
)

# 2. Изолируем файл geant_externals.cppm, чтобы управлять его приоритетом
set(GEANT_EXT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/externals/geant_externals.cppm")

# На случай, если он лежит в src/, проверим его наличие и исключим из общего списка
if(EXISTS "${GEANT_EXT_FILE}")
    list(REMOVE_ITEM ALL_MODULE_INTERFACES "${GEANT_EXT_FILE}")
else()
    # Если путь отличается, попробуем найти его динамически
    file(GLOB_RECURSE GEANT_EXT_FILE_SEARCH "*/geant_externals.cppm")
    if(GEANT_EXT_FILE_SEARCH)
        set(GEANT_EXT_FILE ${GEANT_EXT_FILE_SEARCH})
        list(REMOVE_ITEM ALL_MODULE_INTERFACES ${GEANT_EXT_FILE})
    else()
        message(FATAL_ERROR "Файл geant_externals.cppm не найден! Проверь путь к файлу.")
    endif()
endif()

# 3. Находим все обычные файлы реализации (.cpp)
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")


# --- Создание таргета библиотеки ---
# Для современных C++ модулей достаточно объявить таргет, а исходники передать через target_sources
add_library(GeantCoreLib STATIC)

# --- Распределение файлов по типам (Критический шаг для Ninja dyndep) ---
target_sources(GeantCoreLib
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ${GEANT_EXT_FILE}         # Сначала строго корневой модуль-обертка externals
            ${ALL_MODULE_INTERFACES}  # Затем все остальные модули, зависящие от него
    PRIVATE
        ${ALL_SOURCES}                # Обычные файлы реализации (.cpp)
)

# --- Настройка линковки и зависимостей таргета ---
target_link_libraries(GeantCoreLib PUBLIC
        ${Geant4_LIBRARIES}
        nlohmann_json::nlohmann_json
)

# Инкулуды и дефайны Geant4 пробрасываем в таргет, чтобы geant_externals.cppm их видел
target_include_directories(GeantCoreLib PUBLIC ${Geant4_INCLUDE_DIRS})
target_compile_definitions(GeantCoreLib PUBLIC ${Geant4_DEFINITIONS})