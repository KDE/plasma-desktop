add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.pager\")
plasma_install_package(package org.kde.plasma.pager)

set(pager_SRCS
    plugin/pagermodel.cpp
    plugin/pagerplugin.cpp
    plugin/windowmodel.cpp)

add_library(pagerplugin SHARED ${pager_SRCS})

target_link_libraries(pagerplugin
                      KF6::KIOCore
                      KF6::WindowSystem
                      KF6::Plasma
                      Plasma::Activities
                      PW::LibTaskManager
                      Qt::Core
                      Qt::DBus
                      Qt::Qml
                      Qt::Quick)

if (X11_FOUND)
    target_link_libraries(pagerplugin Qt::GuiPrivate)
endif()

install(TARGETS pagerplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/pager)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/pager)
