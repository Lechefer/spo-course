cmake_minimum_required(VERSION 3.25)
project(kursach)

set(CMAKE_CXX_STANDARD 17)

add_executable(admin_panel_executor.cpp admin_panel.h admin_panel.cpp lib.h admin_panel_executor.cpp)

add_executable(terminal_executor.cpp terminal.h terminal.cpp lib.h terminal_executor.cpp)
