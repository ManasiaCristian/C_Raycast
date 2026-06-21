cmake_minimum_required(VERSION 3.15)
project(AsciiRaycaster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_executable(raycaster
    src/main.cpp
    src/GameState.cpp
    src/Raycaster.cpp
    src/Renderer.cpp
    src/Input.cpp
)

target_include_directories(raycaster PRIVATE include)

if(MSVC)
    target_compile_options(raycaster PRIVATE /W4 /O2 /permissive-)
else()
    target_compile_options(raycaster PRIVATE -Wall -Wextra -O2)
    # POSIX build uses std::thread -> needs the threading library.
    find_package(Threads REQUIRED)
    target_link_libraries(raycaster PRIVATE Threads::Threads)
endif()
