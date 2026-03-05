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

# Add source to this project's executable.
add_executable (cinebar-generator 
	"src/cinebar-generator.cpp"  
	"src/logger.h" 
	"src/logger.cpp"  
	"src/parser.h" 
	"src/parser.cpp" 
	"src/video_processor.h" 
	"src/video_processor.cpp")

# Install targets.
target_link_libraries(cinebar-generator PRIVATE
	spdlog::spdlog
	CLI11::CLI11
	${OpenCV_LIBS}
)

# Add tests
enable_testing()

add_executable(cinebar-generator-tests
    "tests/test_video_processor.cpp"
	"tests/test_parser.cpp"
	"src/video_processor.cpp"
	"src/parser.cpp"
)

target_link_libraries(cinebar-generator-tests PRIVATE
    GTest::gtest_main
	CLI11::CLI11
    ${OpenCV_LIBS}
)

include(GoogleTest)
gtest_discover_tests(cinebar-generator-tests)
