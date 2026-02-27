cmake_minimum_required(VERSION 3.20)
project(wolfman_alpha LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(wolfman_alpha
  src/wa_audio.cpp
  src/wa_io.cpp
  src/wa_cpu.cpp
  src/wa_calc.cpp
  src/wa_components.cpp
  src/wa_gui.cpp
)

target_include_directories(wolfman_alpha PUBLIC include)

add_executable(wa_console apps/wa_console.cpp)
target_link_libraries(wa_console PRIVATE wolfman_alpha)

add_executable(wa_gui_app apps/wa_gui_app.cpp)
target_link_libraries(wa_gui_app PRIVATE wolfman_alpha)
