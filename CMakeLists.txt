cmake_minimum_required(VERSION 3.19)

set(grpc_named_pipe_MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(grpc_named_pipe_MAIN_PROJECT ON)
endif()

if(grpc_named_pipe_MAIN_PROJECT)
    # configure vcpkg
    if ("$ENV{VCPKG_ROOT}" STREQUAL "")
    message(FATAL_ERROR "VCPKG_ROOT not found")
    endif()
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(grpc_named_pipe VERSION 0.0.1 LANGUAGES CXX)

message(STATUS "vcpkg installed dir: ${VCPKG_INSTALLED_DIR}")

set(Boost_NO_WARN_NEW_VERSIONS ON)

message(STATUS "fetching winasio")
include(FetchContent)
FetchContent_Declare(winasio
  GIT_REPOSITORY https://github.com/youyuanwu/winasio.git
  GIT_TAG 50a7641f192bfc3bf010777a9a4800fe8e2fdb84)
FetchContent_GetProperties(winasio)
if(NOT winasio_POPULATED)
  FetchContent_Populate(winasio)
  # add subdir is outside and after fmt targets.
  add_subdirectory(${winasio_SOURCE_DIR} ${winasio_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

if(grpc_named_pipe_MAIN_PROJECT)
  include(${winasio_SOURCE_DIR}/cmake/clang-format.cmake)
endif()

# use vcpkg
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

file(GLOB_RECURSE _SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(grpc_named_pipe STATIC ${_SOURCES})

target_include_directories(grpc_named_pipe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(grpc_named_pipe PUBLIC gRPC::grpc++ winasio)

set_property(TARGET grpc_named_pipe PROPERTY CXX_STANDARD 17)

add_subdirectory(examples)
enable_testing()
add_subdirectory(tests)