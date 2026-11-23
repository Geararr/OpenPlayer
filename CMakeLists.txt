#cmake_minimum_required(VERSION 3.15)
#project(OpenPlayer_Slint VERSION 1.0)
#
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
#find_package(ZLIB REQUIRED)
#find_package(TagLib REQUIRED)
#find_package(nlohmann_json CONFIG REQUIRED)
#
#find_package(PkgConfig REQUIRED)
#
#pkg_check_modules(VIPS REQUIRED IMPORTED_TARGET vips-cpp)
#
#if(CMAKE_CROSSCOMPILING OR CMAKE_CXX_COMPILER MATCHES "mingw" OR WIN32)
#    message(STATUS "Cross-compiling for Windows: Fetching windows-compatible Slint sources.")
#
#    if(CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32")
#        message(STATUS "COMPILING FOR WIN-X64")
#        set(TARGET_RUST_TRIPLET "x86_64-pc-windows-gnu")
#        set(CMAKE_RC_COMPILER "x86_64-w64-mingw32-windres")
#
#    elseif(CMAKE_CXX_COMPILER MATCHES "i686-w64-mingw32")
#        message(STATUS "COMPILING FOR WIN-X86")
#        set(TARGET_RUST_TRIPLET "i686-pc-windows-gnu")
#        set(CMAKE_RC_COMPILER "i686-w64-mingw32-windres")
#    endif()
#
#    set(Rust_CARGO_TARGET "${TARGET_RUST_TRIPLET}" CACHE STRING "" FORCE)
#    set(RUST_CARGO_TARGET "${TARGET_RUST_TRIPLET}" CACHE STRING "" FORCE)
#    set(CARGO_BUILD_TARGET "${TARGET_RUST_TRIPLET}" CACHE STRING "" FORCE)
#
#    include(FetchContent)
#    FetchContent_Declare(
#            Slint
#            GIT_REPOSITORY https://github.com/slint-ui/slint
#            GIT_TAG v1.18.0
#            SOURCE_SUBDIR api/cpp
#    )
#
#    set(SLINT_FEATURE_BACKEND_QT OFF CACHE BOOL "" FORCE)
#    set(SLINT_FEATURE_BACKEND_WINIT ON CACHE BOOL "" FORCE)
#    set(SLINT_FEATURE_RENDERER_SKIA OFF CACHE BOOL "" FORCE)
#
#    FetchContent_MakeAvailable(Slint)
#else()
#    message(STATUS "Native linux build: Using local Slint path.")
#    find_package(Slint REQUIRED NO_DEFAULT_PATH PATHS "/usr/lib/cmake/Slint")
#endif()
#
#add_executable(OpenPlayer
#        src/main.cpp
#        src/uiManager.cpp
#        src/soundManager.cpp
#        src/utils.cpp
#        src/eventManager.cpp
#        src/jsonManager.cpp
#        src/miniaudio_impl.cpp
#)
#
#include(CheckIPOSupported)
#check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
#if(ipo_supported)
#    set_property(TARGET OpenPlayer PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
#endif()
#
#target_compile_options(OpenPlayer PRIVATE
#    $<$<CONFIG:Release>:-O3 -DNDEBUG>
#)
#
#target_link_options(OpenPlayer PRIVATE
#    $<$<CONFIG:Release>:-s>
#)
#
#if(CMAKE_CROSSCOMPILING OR CMAKE_CXX_COMPILER MATCHES "mingw" OR WIN32)
#    enable_language(RC)
#    target_sources(OpenPlayer PRIVATE assets/win_icon.rc)
#
#    set(LINKS_DIR "${CMAKE_CURRENT_BINARY_DIR}/slint_headers")
#    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${LINKS_DIR}")
#    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
#            "${CMAKE_CURRENT_BINARY_DIR}/_deps/slint-src/api/cpp/include"
#            "${LINKS_DIR}/slint"
#    )
#
#    target_include_directories(OpenPlayer PRIVATE
#            "${LINKS_DIR}"
#            "${CMAKE_CURRENT_BINARY_DIR}/_deps/slint-build/generated_include"
#    )
#endif()
#
#set_property(TARGET OpenPlayer PROPERTY SLINT_EMBED_RESOURCES embed-files)
#
#slint_target_sources(OpenPlayer include/ui/app_window.slint)
#target_include_directories(OpenPlayer PRIVATE include)
#
#target_link_libraries(OpenPlayer PRIVATE
#        Slint::Slint
#        TagLib::tag
#        nlohmann_json::nlohmann_json
#        PkgConfig::VIPS
#)
#
#if(CMAKE_CROSSCOMPILING OR CMAKE_CXX_COMPILER MATCHES "mingw" OR WIN32)
#    target_link_libraries(OpenPlayer PRIVATE winmm)
#    set_target_properties(OpenPlayer PROPERTIES WIN32_EXECUTABLE TRUE)
#
#    if(MINGW OR CMAKE_CXX_COMPILER MATCHES "mingw")
#        target_link_options(OpenPlayer PRIVATE
#                "-mwindows"
#                "-Wl,--subsystem,windows"
#                "-static-libgcc"
#                "-static-libstdc++"
#        )
#    endif()
#endif()

cmake_minimum_required(VERSION 3.15)
project(OpenPlayer_Slint VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(TagLib REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(VIPS REQUIRED IMPORTED_TARGET vips-cpp)

include(FetchContent)
FetchContent_Declare(
        Slint
        GIT_REPOSITORY https://github.com/slint-ui/slint
        GIT_TAG v1.18.0
        SOURCE_SUBDIR api/cpp
)

set(SLINT_FEATURE_BACKEND_QT OFF CACHE BOOL "" FORCE)
set(SLINT_FEATURE_BACKEND_WINIT ON CACHE BOOL "" FORCE)
set(SLINT_FEATURE_RENDERER_SKIA OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Slint)

add_executable(OpenPlayer
        src/main.cpp
        src/uiManager.cpp
        src/soundManager.cpp
        src/utils.cpp
        src/eventManager.cpp
        src/jsonManager.cpp
        src/miniaudio_impl.cpp
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set_property(TARGET OpenPlayer PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

target_compile_options(OpenPlayer PRIVATE
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_link_options(OpenPlayer PRIVATE
        $<$<CONFIG:Release>:-s>
)

set_property(TARGET OpenPlayer PROPERTY SLINT_EMBED_RESOURCES embed-files)

slint_target_sources(OpenPlayer include/ui/app_window.slint)
target_include_directories(OpenPlayer PRIVATE include)

target_link_libraries(OpenPlayer PRIVATE
        Slint::Slint
        TagLib::tag
        nlohmann_json::nlohmann_json
        PkgConfig::VIPS
)

if(WIN32)
    enable_language(RC)
    target_sources(OpenPlayer PRIVATE assets/win_icon.rc)
    target_link_libraries(OpenPlayer PRIVATE winmm)
    set_target_properties(OpenPlayer PROPERTIES WIN32_EXECUTABLE TRUE)

    if(MINGW OR CMAKE_CXX_COMPILER MATCHES "mingw")
        target_link_options(OpenPlayer PRIVATE
                "-mwindows"
                "-Wl,--subsystem,windows"
                "-static-libgcc"
                "-static-libstdc++"
        )
    endif()
endif()