cmake_minimum_required(VERSION 3.15)
project(OpenPlayer_Slint VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(TagLib REQUIRED)

if(CMAKE_CROSSCOMPILING OR CMAKE_CXX_COMPILER MATCHES "mingw" OR WIN32)
    message(STATUS "Cross-compiling for Windows: Fetching windows-compatible Slint sources.")

    set(Rust_CARGO_TARGET "x86_64-pc-windows-gnu" CACHE STRING "" FORCE)
    set(RUST_CARGO_TARGET "x86_64-pc-windows-gnu" CACHE STRING "" FORCE)
    set(CARGO_BUILD_TARGET "x86_64-pc-windows-gnu" CACHE STRING "" FORCE)

    include(FetchContent)
    FetchContent_Declare(
            Slint
            GIT_REPOSITORY https://github.com/slint-ui/slint
            GIT_TAG v1.18.0
            SOURCE_SUBDIR api/cpp
    )
    FetchContent_MakeAvailable(Slint)
else()
    message(STATUS "Native linux build: Using local Slint path.")
    find_package(Slint REQUIRED NO_DEFAULT_PATH PATHS "/usr/lib/cmake/Slint")
endif()

add_executable(OpenPlayer
        src/main.cpp
        src/uiManager.cpp
        src/soundManager.cpp
        src/utils.cpp
        src/eventManager.cpp
        src/miniaudio_impl.cpp
)

if(CMAKE_CROSSCOMPILING OR CMAKE_CXX_COMPILER MATCHES "mingw" OR WIN32)
    set(LINKS_DIR "${CMAKE_CURRENT_BINARY_DIR}/slint_headers")

    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${LINKS_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_CURRENT_BINARY_DIR}/_deps/slint-src/api/cpp/include"
            "${LINKS_DIR}/slint"
    )

    target_include_directories(OpenPlayer PRIVATE "${LINKS_DIR}")
    target_include_directories(OpenPlayer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/_deps/slint-build/generated_include")
endif()


slint_target_sources(OpenPlayer include/ui/app_window.slint)
target_include_directories(OpenPlayer PRIVATE include)

target_link_libraries(OpenPlayer PRIVATE
        Slint::Slint
        TagLib::tag
)

if(WIN32)
    target_link_libraries(OpenPlayer PRIVATE winmm)
    set_target_properties(OpenPlayer PROPERTIES WIN32_EXECUTABLE TRUE)

    if(MINGW)
        target_link_options(OpenPlayer PRIVATE "-mwindows" "-static-libgcc" "-static-libstdc++")
    endif()
endif()
