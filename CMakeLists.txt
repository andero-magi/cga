cmake_minimum_required(VERSION 3.17)

project(CGA
    VERSION 1.0
    DESCRIPTION "Attempt at making a game in C"
    LANGUAGES C)

find_package(OpenGL REQUIRED)
#find_package(glfw3 REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR} include/)

add_executable(game
  src/main.c
)

target_link_libraries(game ${OPENGL_gl_LIBRARY} glfw)