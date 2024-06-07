cmake_minimum_required(VERSION 3.17)

project(CGA
    VERSION 1.0
    DESCRIPTION "Attempt at making a game in C"
    LANGUAGES C)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glew-cmake)

include_directories(${OPENGL_INCLUDE_DIR} include/)

add_executable(game
  src/main.c
  src/cga_window.h
  src/cga_window.c
  src/cga_core.h
  src/cga_core.c
  src/cga_inputs.h
  src/game.c
  src/game.h
  src/glutil.h
  src/glutil.c
  src/font_draw.h
  src/font_draw.c
  src/log.c
  src/log.h
  src/cga_render.h
  src/cga_render.c
)

target_link_libraries(game ${OPENGL_gl_LIBRARY} glfw libglew_static)

target_include_directories(game PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/glew-cmake/include
)

set_target_properties(game
    PROPERTIES
    C_STANDARD 17
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
)