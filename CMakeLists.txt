cmake_minimum_required(VERSION 3.0.0)
project(GameOfLife VERSION 1.0)

include(CTest)
enable_testing()

add_executable(GameOfLife ./src/main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(OpenGL_GL_PREFERENCE GLVND)
include(CPack)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(DevIL REQUIRED)

include_directories( ${DevIL_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} )
target_link_libraries(GameOfLife ${OPENGL_LIBRARIES} glfw ${GLEW_LIBRARY} ${DevIL_LIBRARY} )
