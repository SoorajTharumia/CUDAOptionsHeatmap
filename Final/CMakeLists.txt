cmake_minimum_required(VERSION 3.18)

project(BlackScholesHeatmap LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-O2>)
add_compile_options($<$<COMPILE_LANGUAGE:CUDA>:-O2>)

set(CMAKE_CUDA_ARCHITECTURES native)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin")

find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.cu")

add_executable(FinalProject ${SOURCES})

target_include_directories(FinalProject PRIVATE ${CMAKE_SOURCE_DIR}/src)
include_directories(
    ../external/glfw-3.1.2/include/
    ../external/glm-0.9.7.1/
    ../external/glew-1.13.0/include/
    .
)

add_compile_definitions(
    GLEW_STATIC
    _CRT_SECURE_NO_WARNINGS
)

set(ALL_LIBS
    OpenGL::GL
    glfw
    GLEW_1130
    sfml-network
    sfml-system
)

target_link_libraries(FinalProject PRIVATE ${ALL_LIBS})

if(UNIX AND NOT APPLE)
    target_link_libraries(FinalProject PRIVATE dl pthread)
endif()

set(SHADERS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")

file(GLOB_RECURSE SHADERS_FILES
    CONFIGURE_DEPENDS
    "${SHADERS_SOURCE_DIR}/*"
)

set_target_properties(FinalProject PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

add_custom_target(copy_shaders ALL
    COMMENT "Copying shaders to build directory..."
)

foreach(SRC_FILE ${SHADERS_FILES})
    file(RELATIVE_PATH REL_PATH "${SHADERS_SOURCE_DIR}" "${SRC_FILE}")
    set(DST_FILE "${SHADERS_DEST_DIR}/${REL_PATH}")
    get_filename_component(DST_DIR "${DST_FILE}" DIRECTORY)

    add_custom_command(
        TARGET copy_shaders PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE}" "${DST_FILE}"
        COMMENT "Copying ${REL_PATH}"
    )
endforeach()

add_dependencies(FinalProject copy_shaders)