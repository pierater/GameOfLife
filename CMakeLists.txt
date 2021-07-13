cmake_minimum_required(VERSION 3.19)
project(Life)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Life
        Driver.cpp
        )

#For the Curses library to load:
find_package(Curses REQUIRED)
SET(CURSES_USE_NCURSES TRUE)
target_link_libraries(Life ${CURSES_LIBRARIES})
