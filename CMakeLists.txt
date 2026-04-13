# CMakeList.txt : CMake project for cinebar-generator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("cinebar-generator")

# Add packages
find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)

include(FetchContent)

FetchContent_Declare(
  indicators
  GIT_REPOSITORY https://github.com/p-ranav/indicators.git
  GIT_TAG master
)

FetchContent_MakeAvailable(indicators)

# Library (shared)
add_library(cinebar-lib
    src/cinebar_generator.cpp
    src/video_processor.cpp
    src/frame_extractor.cpp
    src/parser.cpp
    src/logger.cpp
    src/utility.cpp
)

target_include_directories(cinebar-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cinebar-lib PUBLIC
    spdlog::spdlog
    CLI11::CLI11
    ${OpenCV_LIBS}
    indicators
)

# Main executable
add_executable(cinebar-generator
    src/main.cpp
)

target_link_libraries(cinebar-generator PRIVATE
    cinebar-lib
)

# Tests
enable_testing()

add_executable(cinebar-generator-tests
    tests/test_video_processor.cpp
    tests/test_frame_extractor.cpp
    tests/test_cinebar_generator.cpp
    tests/test_parser.cpp
)

target_link_libraries(cinebar-generator-tests PRIVATE
    cinebar-lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cinebar-generator-tests)