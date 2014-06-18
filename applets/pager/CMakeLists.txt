add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.pager\")
plasma_install_package(package org.kde.plasma.pager)

set(pager_SRCS
    plugin/model.cpp
    plugin/pager.cpp
    plugin/pagerplugin.cpp)

add_library(pagerplugin SHARED ${pager_SRCS})

target_link_libraries(pagerplugin
                      KF5::KIOCore
                      KF5::WindowSystem
                      KF5::Plasma
                      KF5::Activities
                      KF5::KDELibs4Support
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::DBus
                      PW::LibTaskManager)

if (X11_FOUND)
    target_link_libraries(pagerplugin Qt5::X11Extras)
endif()

install(TARGETS pagerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/pager)
install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/pager)
