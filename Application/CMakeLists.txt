cmake_minimum_required(VERSION 3.28)

add_executable(GeantApp
    src/main.cpp
)

target_link_libraries(GeantApp PRIVATE GeantCoreLib)