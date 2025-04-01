plasma_install_package(package org.kde.plasma.showdesktop)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.showdesktop")

ecm_add_qml_module(showdesktopplugin
    URI org.kde.plasma.private.showdesktop
    VERSION 0.1
    GENERATE_PLUGIN_SOURCE
)

qt_add_dbus_interface(showdesktop_SRCS org.kde.KWin.xml kwin_interface)

target_sources(showdesktopplugin PRIVATE
    showdesktop.cpp
    ${showdesktop_SRCS}
)

target_link_libraries(showdesktopplugin PRIVATE
     Qt::Core
     Qt::DBus
     Qt::Qml
     Qt::Quick
     KF6::WindowSystem
    )

ecm_finalize_qml_module(showdesktopplugin)
