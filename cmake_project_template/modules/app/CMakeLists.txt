cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(app
	VERSION 0.0.1
	LANGUAGES CXX
)

if (x86)
	add_definitions(-Dx86)
endif()
if (FEATURE_BAR)
	add_definitions(-DFEATURE_BAR)
endif()

add_executable(${PROJECT_NAME}
	app.cpp
)

target_link_libraries(${PROJECT_NAME} libfoo)

target_link_options(${PROJECT_NAME} PRIVATE -Wl,-rpath=$ORIGIN)

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${DEPLOY_DIR}"
)

# Packaging
include(GNUInstallDirs)
install(
	TARGETS ${PROJECT_NAME}
	EXPORT "${PROJECT_NAME}Targets"
	COMPONENT ${PROJECT_NAME}
)