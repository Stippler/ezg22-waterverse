cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)
project(Engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

# find OpenGL
find_package(OpenGL REQUIRED)

# GLFW configuration
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)

# Assimp configuration
set(ZLIB_LIBRARIES zlibstatic)
set(BUILD_STATIC_LIB ON)
set(BUILD_ASSIMP_TOOLS  ON)
set(ASSIMP_BUILD_STATIC_LIB ON)

add_subdirectory(external/assimp)
add_subdirectory(external/glew/build/cmake)
add_subdirectory(external/glfw)

message(STATUS "Installing glm via submodule")
set(BUILD_STATIC_LIBS ON)
add_subdirectory(external/glm)
set_property(TARGET glm_static PROPERTY FOLDER "Dependencies")

set_property(TARGET glfw PROPERTY FOLDER "Dependencies")


file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

# Solution Filters
foreach(source IN LISTS FILE_SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    string(REPLACE "src" "" source_path_final "${source_path_msvc}")
    source_group("${source_path_final}" FILES "${source}")
endforeach()

add_executable(
    ${PROJECT_NAME}
    "${FILE_SOURCES}"
)

target_include_directories( 
    ${PROJECT_NAME} PUBLIC
    external/assimp/include
    external/glfw/include
    external/glew/include
    external/glm
    ${OPENGL_INCLUDE_DIRS}
)

target_link_directories(
    ${PROJECT_NAME} PRIVATE 
    external/glfw/src
)

target_link_libraries(
    ${PROJECT_NAME}
    assimp
    glfw
    glew_s
    glm::glm
    ${OPENGL_LIBRARIES}
)

# Change working directory to top dir to access `assets/shaders/` folder
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/..)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()
