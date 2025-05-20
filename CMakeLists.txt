cmake_minimum_required(VERSION 3.10)
project(gridfire LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

# Find glslc
find_program(GLSLC glslc)
if (NOT GLSLC)
    message(FATAL_ERROR "glslc not found! Please install Vulkan SDK or vulkan-tools.")
endif()

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/src/shaders)
set(SHADER_FILES
    ${SHADER_DIR}/raymarch.vert
    ${SHADER_DIR}/raymarch.frag
)

foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${CMAKE_BINARY_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME}"
    )
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_OUTPUTS})

add_executable(gridfire
    src/main.cpp
    src/device.cpp
    src/swapchain.cpp
    src/pipeline.cpp
    src/input.cpp
)

add_dependencies(gridfire shaders)

target_include_directories(gridfire PRIVATE src)
target_link_libraries(gridfire Vulkan::Vulkan glfw glm::glm)

# Copy compiled shaders to binary directory
add_custom_command(TARGET gridfire POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SHADER_OUTPUTS}
    $<TARGET_FILE_DIR:gridfire>
    COMMENT "Copying SPIR-V shaders to binary directory"
)
