cmake_minimum_required(VERSION 3.28)
project(GeantApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(G4CoreLib)
add_subdirectory(Application)



