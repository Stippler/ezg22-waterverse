cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)
project(Engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

# GLFW
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)

# set(GLFW_VULKAN_STATIC ON)
add_subdirectory(external/glfw)
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

find_package(OpenGL REQUIRED)

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC external/glfw/include
    ${OPENGL_INCLUDE_DIRS}
)

target_link_directories(${PROJECT_NAME}
    PRIVATE external/glfw/src
)

target_link_libraries(
    ${PROJECT_NAME}
    glfw
    ${OPENGL_LIBRARIES}
)

# Change working directory to top dir to access `assets/shaders/` folder
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/..)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()
