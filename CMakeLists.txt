cmake_minimum_required(VERSION 3.10)
project(Emulator)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(Threads REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${SFML_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/darknet/include)

file(GLOB SOURCES "src/*.cpp")

add_executable(emulator ${SOURCES})

target_link_libraries(emulator ${OpenCV_LIBS} sfml-graphics sfml-window sfml-system ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_SOURCE_DIR}/darknet/libdarknet.so -lespeak -lasound)

