add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.kickoff\")

#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.kickoff)


#######################################################################################
# Kickoff Library

set(libkickoff_SRCS
    core/applicationmodel.cpp
    core/favoritesmodel.cpp
    core/itemhandlers.cpp
    core/kickoffabstractmodel.cpp
    core/kickoffmodel.cpp
    core/kickoffproxymodel.cpp
    core/kickoffplugin.cpp
    core/krunnermodel.cpp
    core/leavemodel.cpp
    core/models.cpp
    core/recentapplications.cpp
    core/recentlyusedmodel.cpp
    core/systemmodel.cpp
    core/urlitemlauncher.cpp
    core/processrunner.cpp
)

qt5_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.xml core/applicationmodel.h Kickoff::ApplicationModel)
qt5_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.recent.xml core/recentlyusedmodel.h Kickoff::RecentlyUsedModel)

qt5_add_dbus_interface(libkickoff_SRCS ${SCREENSAVER_DBUS_INTERFACE} screensaver_interface)
qt5_add_dbus_interface(libkickoff_SRCS ${KRUNNERAPP_INTERFACE} krunner_interface)
qt5_add_dbus_interface(libkickoff_SRCS ${KSMSERVER_DBUS_INTERFACE} ksmserver_interface)

install(FILES core/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kickoff)
add_library(kickoffplugin SHARED ${libkickoff_SRCS})
target_link_libraries(kickoffplugin
                              Qt5::Qml
                              Qt5::DBus
                              Qt5::X11Extras
                              KF5::I18n
                              KF5::KIOFileWidgets
                              KF5::Runner
                              KF5::Solid
                              KF5::Service
                              KF5::KDELibs4Support
                              KF5::WindowSystem
                              PW::KWorkspace)
install(TARGETS kickoffplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kickoff)

