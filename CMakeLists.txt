cmake_minimum_required(VERSION 3.7)
project("test")

find_package(Eigen3 3.3 REQUIRED)
find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIRS} ${SFML_INCLUDE_DIRS})

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})

add_executable(test src/main.cpp src/scene.cpp)
target_compile_definitions(test PRIVATE SFML_STATIC)
TARGET_LINK_LIBRARIES(test sfml-system sfml-window sfml-graphics sfml-audio)

