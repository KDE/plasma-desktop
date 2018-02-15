set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_subdirectory(backend)

plasma_install_package(package org.kde.plasma.kimpanel)

set(kimpanelplugin_SRCS
    plugin/screen.cpp
    plugin/kimpanelplugin.cpp
)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kimpanel)
add_library(kimpanelplugin SHARED ${kimpanelplugin_SRCS})

target_link_libraries(kimpanelplugin Qt5::Gui Qt5::Qml)
install(TARGETS kimpanelplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kimpanel)
