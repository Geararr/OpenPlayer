cmake_minimum_required(VERSION 3.15)
project(OpenPlayer_Slint VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Slint REQUIRED)
find_package(Taglib 2.0.0 REQUIRED)

add_executable(OpenPlayer
        src/main.cpp
        src/uiManager.cpp
        src/soundManager.cpp
        src/utils.cpp
        src/eventManager.cpp
        src/miniaudio_impl.cpp
)

slint_target_sources(OpenPlayer include/ui/app_window.slint)

target_include_directories(OpenPlayer PRIVATE include)

if(UNIX AND NOT APPLE)
    target_link_libraries(OpenPlayer PRIVATE
            Slint::Slint
            pthread
            m
            dl
            TagLib::tag
    )
elseif(WIN32)
    target_link_libraries(OpenPlayer PRIVATE
            Slint::Slint
            winmm
            TagLib::tag
    )

    set_target_properties(OpenPlayer PROPERTIES
            WIN32_EXECUTABLE TRUE
    )

    if(MSVC)
        target_link_options(OpenPlayer PRIVATE "/ENTRY:mainCRTStartup")
        target_link_options(OpenPlayer PRIVATE "/SUBSYSTEM:WINDOWS")
    elseif(MINGW)
        target_link_options(OpenPlayer PRIVATE
                "-mwindows"
                "-static-libgcc"
                "-static-libstdc++"
                "-Wl,-Bstatic,-lwinpthread,-Bdynamic"
        )
    endif()
endif()
