cmake_minimum_required(VERSION 3.28)

add_executable(GeantApp
    src/main.cpp
)

target_link_libraries(GeantApp PRIVATE GeantCoreLib)

# Добавляем шаг копирования после успешной сборки таргета GeantApp
add_custom_command(TARGET GeantApp POST_BUILD
    # Используем встроенную в CMake кроссплатформенную утилиту для копирования папок
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        # Откуда копируем: полный путь до папки macroses в библиотеке
        "${CMAKE_SOURCE_DIR}/G4CoreLib/src/macroses"
        # Куда копируем: создаем папку AppConfigs рядом с собранным .exe / бинарником
        "$<TARGET_FILE_DIR:GeantApp>/AppConfigs"
    COMMENT "Копирование макросов Geant4 в директорию сборки..."
)