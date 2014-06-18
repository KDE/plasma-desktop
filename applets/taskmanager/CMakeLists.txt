add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.taskmanager\")

plasma_install_package(package org.kde.plasma.taskmanager)

set(taskmanagerplugin_SRCS
    plugin/backend.cpp
    plugin/draghelper.cpp
    plugin/taskmanagerplugin.cpp
    plugin/textlabel.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/taskmanager)

add_library(taskmanagerplugin SHARED ${taskmanagerplugin_SRCS})

target_link_libraries(taskmanagerplugin
                      PW::LibTaskManager
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::KIOCore
                      KF5::WindowSystem)

install(TARGETS taskmanagerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/taskmanager)
