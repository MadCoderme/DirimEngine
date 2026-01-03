cmake_minimum_required(VERSION 3.10)
project(DirimEngine VERSION 1.0)

# 1. Find GLFW using the common package name on Linux systems
# This finds the necessary headers and linking information for glfw3
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
# 2. Find SDL2
# This finds the necessary headers and linking information for SDL2
find_package(SDL2 REQUIRED)

find_package(assimp REQUIRED)

find_package(Freetype REQUIRED)

link_directories(${CMAKE_SOURCE_DIR}/lib)

# 3. Add Source Files
add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/stb_image.cpp
    vendor/src/glad.c
)

# 4. Include Directories
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    vendor/include
    # Include both sets of headers using the variables provided by the find_package calls
    ${GLFW_INCLUDE_DIRS} 
    ${SDL2_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

# 5. Link Libraries
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    # Link to the GLFW library (usually registered as 'glfw' or 'glfw3')
    glfw # Use 'glfw' which is the common imported target name
    
    # Link to the SDL2 libraries (main and OpenGL/system components)
    ${SDL2_LIBRARIES}
    ${SDL2_opengl_LIBRARY} 
    
    # Link to the system OpenGL library
    OpenGL::GL

    assimp::assimp

    Freetype::Freetype

    IrrKlang
)

file(COPY ${CMAKE_SOURCE_DIR}/lib/libIrrKlang.so DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/lib/ikpMP3.so DESTINATION ${CMAKE_BINARY_DIR})