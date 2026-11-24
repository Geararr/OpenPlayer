cmake_minimum_required(VERSION 3.21)
project(slint_wrapper LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        slint
        URL https://github.com/slint-ui/slint/archive/refs/tags/v1.18.0.zip
)
set(SLINT_FEATURE_BACKEND_QT OFF)
set(SLINT_FEATURE_BACKEND_WINIT ON)
set(SLINT_FEATURE_RENDERER_SKIA OFF)
FetchContent_MakeAvailable(slint)