plasma_install_package(package org.kde.plasma.taskmanager)

set(taskmanagerplugin_SRCS
    plugin/backend.cpp
    plugin/draghelper.cpp
    plugin/taskmanagerplugin.cpp
    plugin/textlabel.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/taskmanager)

add_library(taskmanagerplugin SHARED ${taskmanagerplugin_SRCS})

# FIXME TODO HACK: The last two are temp, see comment in Backend::activateItem().
target_link_libraries(taskmanagerplugin
                      PW::LibTaskManager
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::KIOCore
                      KF5::WindowSystem
                      Qt5::X11Extras
                      ${X11_LIBRARIES})

install(TARGETS taskmanagerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/taskmanager)
