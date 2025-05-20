# Specify minimum CMake version and project details
cmake_minimum_required(VERSION 3.16)
project(gridfire
    VERSION 1.0.0
    DESCRIPTION "A Vulkan-based raymarching game"
    LANGUAGES CXX
)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Installation settings
set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation prefix for binaries and resources")
set(INSTALL_SHADER_DIR "${CMAKE_INSTALL_PREFIX}/share/gridfire/shaders" CACHE PATH "Directory for shader installation")
set(INSTALL_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/gridfire" CACHE PATH "Directory for configuration files")

# User-configurable options
option(ENABLE_VULKAN_VALIDATION "Enable Vulkan validation layers for debugging" OFF)
option(ENABLE_VERBOSE_BUILD "Enable verbose build output for debugging" OFF)
option(BUILD_TESTING "Build unit tests (if available)" OFF)

# Find dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

# Find glslc shader compiler
find_program(GLSLC glslc REQUIRED
    HINTS "${Vulkan_GLSLC_EXECUTABLE}"
    DOC "Path to glslc shader compiler"
)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found! Please install the Vulkan SDK or vulkan-tools.")
endif()

# Configure verbose builds if enabled
if(ENABLE_VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Show detailed build output" FORCE)
endif()

# Shader compilation
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/src/shaders")
set(SHADER_FILES
    "${SHADER_DIR}/raymarch.vert"
    "${SHADER_DIR}/raymarch.frag"
)

foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT "${CMAKE_BINARY_DIR}/${SHADER_NAME}.spv")
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${SHADER_NAME}"
        VERBATIM
    )
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_OUTPUTS})

# Define the main executable
add_executable(gridfire
    src/main.cpp
    src/device.cpp
    src/swapchain.cpp
    src/pipeline.cpp
    src/input.cpp
)

# Ensure shaders are built before the executable
add_dependencies(gridfire shaders)

# Set target properties: include directories, compile definitions, and libraries
target_include_directories(gridfire PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}" # For gridfire_config.h
)
target_compile_definitions(gridfire PRIVATE
    USE_VULKAN_VALIDATION=$<BOOL:${ENABLE_VULKAN_VALIDATION}>
)
target_link_libraries(gridfire PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)

# Generate configuration header
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/gridfire_config.in"
    "${CMAKE_BINARY_DIR}/gridfire_config.h"
    @ONLY
)

# Copy compiled shaders to binary directory for development
add_custom_command(TARGET gridfire POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:gridfire>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_OUTPUTS} "$<TARGET_FILE_DIR:gridfire>/shaders"
    COMMENT "Copying SPIR-V shaders to binary directory for development"
)

# Installation rules
install(TARGETS gridfire
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
    COMPONENT Runtime
)
install(FILES ${SHADER_OUTPUTS}
    DESTINATION "${INSTALL_SHADER_DIR}"
    COMPONENT Runtime
)
install(FILES "${CMAKE_SOURCE_DIR}/resources/settings.ini"
    DESTINATION "${INSTALL_CONFIG_DIR}"
    COMPONENT Runtime
)
install(FILES "${CMAKE_BINARY_DIR}/gridfire_config.h"
    DESTINATION "${CMAKE_BINARY_DIR}"
    COMPONENT Runtime
)

# Optionally include testing
if(BUILD_TESTING)
    enable_testing()
    # Add test subdirectory or test files if available, e.g.:
    # add_subdirectory(tests)
endif()
