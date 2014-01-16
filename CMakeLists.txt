project(kcm-touchpad)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH}
)
set(CMAKE_INCLUDE_CURRENT_DIR true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(TOUCHPAD_KCM_VERSION "0.1")

find_package(KDE4 REQUIRED)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND KDE4_KDECONFIG_EXECUTABLE)
    execute_process(COMMAND ${KDE4_KDECONFIG_EXECUTABLE} --prefix
                    OUTPUT_VARIABLE _DETECTED_INSTALL_PREFIX
    )
    if (_DETECTED_INSTALL_PREFIX)
        set(CMAKE_INSTALL_PREFIX ${_DETECTED_INSTALL_PREFIX}
            CACHE PATH "" FORCE
        )
    endif()
endif()

include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})

add_subdirectory(src)
add_subdirectory(icon)
