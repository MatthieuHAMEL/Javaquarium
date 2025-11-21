cmake_minimum_required(VERSION 4.1)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444") # from cmake 4.1.2
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_COMPILER "/opt/gcc-15.2/bin/g++")
set(CMAKE_C_COMPILER "/opt/gcc-15.2/bin/gcc") # to compile C (flecs)

set(CMAKE_CXX_FLAGS -fdiagnostics-color=always)
set(CMAKE_C_FLAGS -fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_MODULE_STD 1)
#set(CMAKE_CXX_EXTENSIONS ON)

project(javaquarium LANGUAGES C CXX)

# collect all sources
file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.ixx")
file(GLOB_RECURSE CPP_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE C_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")

add_executable(javaquarium)

set_property(TARGET javaquarium PROPERTY C_STANDARD 99)

target_sources(javaquarium
  PRIVATE ${CPP_SOURCES} ${C_SOURCES}
  PRIVATE FILE_SET cxx_modules TYPE CXX_MODULES FILES ${MODULE_SOURCES}
)

target_include_directories(javaquarium PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Compiler options : for C++, CMake will add -g/-O*/-DNDEBUG etc based on CMAKE_BUILD_TYPE.
set_property(SOURCE ${CPP_SOURCES} APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror=return-type)

# The C code is always optimized, asserts disabled
set_property(SOURCE ${C_SOURCES} APPEND PROPERTY COMPILE_OPTIONS -Wall -O3 -DNDEBUG)
