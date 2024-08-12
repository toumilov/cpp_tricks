cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(libfoo
	VERSION 0.0.1
	DESCRIPTION "The foo library"
	LANGUAGES CXX
)

option(FEATURE_BAR "Some feature" OFF)
if (FEATURE_BAR)
	message("Bar feature enabled")
	add_definitions(-DFEATURE_BAR)
endif()

set(SOURCES
	src/foo.cpp
)

# Shared
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME}
	PRIVATE
		# where the library itself will look for its internal headers
		${CMAKE_CURRENT_SOURCE_DIR}/src
	PUBLIC
		# where top-level project will look for the library's public headers
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		# where external projects will look for the library's public headers
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${DEPLOY_DIR}")

# Static
add_library(${PROJECT_NAME}-static STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME}-static
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Tests
if (BUILD_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()

# Packaging
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "include/foo.hpp")
include(GNUInstallDirs)
install(
	TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-static
	EXPORT "${PROJECT_NAME}Targets"
	COMPONENT ${PROJECT_NAME}
	# these get default values from GNUInstallDirs, no need to set them
	#RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # bin
	#LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # lib
	#ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # lib
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)