cmake_minimum_required(VERSION 3.15)
project(VulkanProject C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_program(GLSLC glslc HINTS ${Vulkan_GLSLC_EXECUTABLE} REQUIRED)

include_directories(include)

set(SOURCES
    src/vulkan/main.c
    src/vulkan/core.c
    src/vulkan/display.c
    src/vulkan/pipeline.c
    src/vulkan/renderer.c
)

add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/bin/shader.vert.spv"
    COMMAND ${GLSLC} "${CMAKE_SOURCE_DIR}/shaders/shader.vert" -o "${CMAKE_SOURCE_DIR}/bin/shader.vert.spv"
    DEPENDS "${CMAKE_SOURCE_DIR}/shaders/shader.vert"
)

add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/bin/shader.frag.spv"
    COMMAND ${GLSLC} "${CMAKE_SOURCE_DIR}/shaders/shader.frag" -o "${CMAKE_SOURCE_DIR}/bin/shader.frag.spv"
    DEPENDS "${CMAKE_SOURCE_DIR}/shaders/shader.frag"
)

add_custom_target(CompileShaders ALL DEPENDS 
    "${CMAKE_SOURCE_DIR}/bin/shader.vert.spv" 
    "${CMAKE_SOURCE_DIR}/bin/shader.frag.spv"
)

add_executable(VulkanEngine ${SOURCES})
add_executable(DumbDrm src/drm/dumbdrm.c)

target_link_libraries(VulkanEngine PRIVATE Vulkan::Vulkan)

set_target_properties(VulkanEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")