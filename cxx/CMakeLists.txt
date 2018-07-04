cmake_minimum_required (VERSION 3.1.0)
project(gitstatus)

# Pregenerate step in build, generate constants before compilation
add_executable(gen_const src/gen_const.cc)
# add the command to generate the source code
add_custom_command (
  OUTPUT ${CMAKE_BINARY_DIR}/src/const.h
  COMMAND gen_const ${CMAKE_BINARY_DIR}/src/const.h
  DEPENDS gen_const
)

# Build core project
# add_library(gstat src/lib.cc)
add_library(gstat src/lib.cc ${CMAKE_BINARY_DIR}/src/const.h)
add_executable(gitstatus src/main.cc)
target_link_libraries(gitstatus gstat)

# Third-party library
include(ExternalProject)
ExternalProject_Add(googletest
    PREFIX "${CMAKE_BINARY_DIR}/lib"
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "master"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/lib/installed
)

# Prevent build on all targets build
set_target_properties(googletest PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Define ${CMAKE_INSTALL_...} variables
include(GNUInstallDirs)

# Specify where third-party libraries are located
link_directories(${CMAKE_BINARY_DIR}/lib/installed/${CMAKE_INSTALL_LIBDIR})
include_directories(
    ${CMAKE_BINARY_DIR}/lib/installed/${CMAKE_INSTALL_INCLUDEDIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}  # Make project includes from root
)

# This is required for googletest
find_package(Threads REQUIRED)

# Test
add_executable(tests test/lib_test.cc)
target_link_libraries(tests gstat gtest Threads::Threads)

# Make sure third-party is built before executable
add_dependencies(tests googletest)
set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Add post build running of tests
add_custom_command(
    TARGET tests
    COMMENT "Run google tests post build"
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND "./tests" "--gtest_color=yes"
)

# Flags & compiler
set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -pipe -fexceptions -Winline")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-D_FORTIFY_SOURCE=2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
else()
    add_definitions(-D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Og -Wall -Wextra -Werror=format-security -Wundef -Wshadow -Wunused -Winit-self -Weffc++")
    # GProf: -pg
    # GCov: -fprofile-arcs -ftest-coverage
endif()

# Will export build commands to build dir, for YCM users
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Just for coverage, still not working
# TODO: Make it go!
# set(CMAKE_CXX_FLAGS_COVERAGE "-ggdb -O0 --coverage -fprofile-arcs -ftest-coverage -Wall -Wextra")
# if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    # set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)
    # include(CodeCoverage)
    # APPEND_COVERAGE_COMPILER_FLAGS()
    # setup_target_for_coverage(NAME coverage EXECUTABLE tests DEPENDENCIES googletest)

    # # set(CMAKE_CXX_FLAGS_COVERAGE "-ggdb -O0 --coverage -fprofile-arcs -ftest-coverage -Wall -Wextra")
    # # SET(CMAKE_CXX_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
    # # SET(CMAKE_C_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
# endif(CMAKE_BUILD_TYPE STREQUAL "Coverage")

# TODO: Better way?
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
