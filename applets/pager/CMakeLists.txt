add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.pager\")
plasma_install_package(package org.kde.plasma.pager)

ecm_add_qml_module(pagerplugin
    URI "org.kde.plasma.private.pager"
    VERSION 2.0
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtCore
)

target_sources(pagerplugin PRIVATE
    pagermodel.cpp
    windowmodel.cpp)

target_link_libraries(pagerplugin PRIVATE
                      KF6::WindowSystem
                      Plasma::Plasma
                      Plasma::Activities
                      PW::LibTaskManager
                      Qt::Core
                      Qt::DBus
                      Qt::Qml
                      Qt::Quick)

if (X11_FOUND)
    target_link_libraries(pagerplugin PRIVATE Qt::GuiPrivate)
endif()

ecm_finalize_qml_module(pagerplugin)
