cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(Sample
	DESCRIPTION "Sample CMake project"
	VERSION 0.0.1
	LANGUAGES CXX
)

# Disable rpath for apps
set(CMAKE_SKIP_BUILD_RPATH YES CACHE BOOL "Disable rpath")

# Setup deploy directory
set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")

include(CTest)
add_subdirectory(modules)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
include(Packaging)