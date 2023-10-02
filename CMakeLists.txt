cmake_minimum_required(VERSION 3.00)
project(labwork1)

set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_CXX_STANDARD 23)

add_executable(labwork1
        ErrorMessage.cpp
        ErrorMessage.h
        ArgumentParser.cpp
        ArgumentParser.h
        ConsoleReader.cpp
        ConsoleReader.h
        main.cpp
)

