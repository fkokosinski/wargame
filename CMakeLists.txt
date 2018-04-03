cmake_minimum_required(VERSION 3.9)
project(wargame C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
add_executable(wargame src/main.c src/help.c src/server.c src/client.c src/tui.c src/msg.c)
target_link_libraries(wargame pthread ncurses)