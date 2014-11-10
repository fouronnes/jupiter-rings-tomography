cmake_minimum_required(VERSION 2.8)

project(planetarium)
find_package(OpenCV REQUIRED)

add_executable(planetarium
    src/planetarium.cpp
    )

target_link_libraries(planetarium ${OpenCV_LIBS})

