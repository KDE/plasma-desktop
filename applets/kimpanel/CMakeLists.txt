set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_subdirectory(backend)
add_subdirectory(plugin)

plasma_install_package(package org.kde.plasma.kimpanel)
