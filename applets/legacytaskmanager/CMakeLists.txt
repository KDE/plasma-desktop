add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.legacytaskmanager\")

plasma_install_package(package org.kde.plasma.legacytaskmanager)

set(legacytaskmanagerplugin_SRCS
    plugin/backend.cpp
    plugin/draghelper.cpp
    plugin/legacytaskmanagerplugin.cpp
    plugin/textlabel.cpp

    plugin/smartlaunchers/smartlauncherbackend.cpp
    plugin/smartlaunchers/smartlauncheritem.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/legacytaskmanager)

add_library(legacytaskmanagerplugin SHARED ${legacytaskmanagerplugin_SRCS})

target_link_libraries(legacytaskmanagerplugin
                      PW::LibLegacyTaskManager
                      Qt5::Core
                      Qt5::DBus
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::Activities
                      KF5::ActivitiesStats
                      KF5::I18n
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::Plasma
                      KF5::Service
                      KF5::WindowSystem)

install(TARGETS legacytaskmanagerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/legacytaskmanager)
