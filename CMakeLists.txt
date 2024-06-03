cmake_minimum_required(VERSION 3.17)

set (ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set (LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set (RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

project(CGA
    VERSION 1.0
    DESCRIPTION "Attempt at making a game in C"
    LANGUAGES C)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)

include_directories(${OPENGL_INCLUDE_DIR} include/)

add_executable(game
  src/main.c
  src/cga_window.h
  src/cga_window.c
  src/cga_core.h
  src/cga_inputs.h
  src/game.c
  src/game.h
)

target_link_libraries(game ${OPENGL_gl_LIBRARY} glfw)