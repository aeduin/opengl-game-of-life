cmake_minimum_required(VERSION 2.8)
project(GameOfLife)
add_executable(GameOfLife main.cpp)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(DevIL REQUIRED)

include_directories( ${DevIL_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} )
target_link_libraries(GameOfLife ${OPENGL_LIBRARIES} glfw ${GLEW_LIBRARY} ${DevIL_LIBRARY} )