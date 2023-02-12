plasma_install_package(package org.kde.plasma.showdesktop)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.showdesktop")

set(showdesktop_SRCS
    plugin/showdesktop.cpp
    plugin/showdesktopplugin.cpp
)

qt_add_dbus_interface(showdesktop_SRCS plugin/org.kde.KWin.xml kwin_interface)

add_library(showdesktopplugin SHARED ${showdesktop_SRCS})

target_link_libraries(showdesktopplugin
     Qt::Core
     Qt::DBus
     Qt::Qml
     Qt::Quick
     KF6::WindowSystem
    )

install(TARGETS showdesktopplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/showdesktop)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/showdesktop)
