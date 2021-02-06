plasma_install_package(package org.kde.plasma.showdesktop)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.showdesktop")

set(showdesktop_SRCS
    plugin/showdesktop.cpp
    plugin/showdesktopplugin.cpp
)

add_library(showdesktopplugin SHARED ${showdesktop_SRCS})

target_link_libraries(showdesktopplugin
     Qt::Core
     Qt::Qml
     Qt::Quick
     KF5::WindowSystem
    )

install(TARGETS showdesktopplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/showdesktop)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/showdesktop)
