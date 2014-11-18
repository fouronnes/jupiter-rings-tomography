cmake_minimum_required(VERSION 2.8)

project(planetarium)
find_package(OpenCV REQUIRED)

add_executable(planetarium
    src/planetarium.cpp
    )

add_executable(calibration
    src/calibration.cpp
    )

target_link_libraries(planetarium ${OpenCV_LIBS})
target_link_libraries(calibration ${OpenCV_LIBS})

