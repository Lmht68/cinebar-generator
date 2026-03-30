# CMakeList.txt : CMake project for cinebar-generator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("cinebar-generator")

# Add packages
find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)

# Library (shared)
add_library(cinebar-lib
    src/cinebar_generator.cpp
    src/video_processor.cpp
    src/frame_extractor.cpp
    src/parser.cpp
    src/logger.cpp
)

target_include_directories(cinebar-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cinebar-lib PUBLIC
    spdlog::spdlog
    CLI11::CLI11
    ${OpenCV_LIBS}
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
    tests/test_parser.cpp
)

target_link_libraries(cinebar-generator-tests PRIVATE
    cinebar-lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cinebar-generator-tests)