project(kcm-touchpad)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH}
)
set(CMAKE_INCLUDE_CURRENT_DIR true)
set(TOUCHPAD_KCM_VERSION "0.1")

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})

add_subdirectory(src)
add_subdirectory(icon)
