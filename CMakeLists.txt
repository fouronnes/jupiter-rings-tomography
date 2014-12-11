cmake_minimum_required(VERSION 2.8)

project(planetarium)
find_package(OpenCV REQUIRED)

add_executable(planetarium
    src/planetarium.cpp
    )

add_executable(calibration
    src/calibration.cpp
    )

add_executable(simulation
    src/simulation.cpp
    )

target_link_libraries(planetarium ${OpenCV_LIBS})
target_link_libraries(calibration ${OpenCV_LIBS})
target_link_libraries(simulation ${OpenCV_LIBS})

