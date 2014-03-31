
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
    core/kuserproxy.cpp
    core/urlitemlauncher.cpp
)

qt5_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.xml core/applicationmodel.h Kickoff::ApplicationModel)
qt5_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.recent.xml core/recentlyusedmodel.h Kickoff::RecentlyUsedModel)

if(${CMAKE_SOURCE_DIR} STREQUAL ${plasma-desktop_SOURCE_DIR})
    set(screensaver_xml ${SCREENSAVER_DBUS_INTERFACE})
    set(krunner_xml ${KRUNNERAPP_INTERFACE})
    set(ksmserver_xml ${KSMSERVER_DBUS_INTERFACE})
else()
    set(screensaver_xml ${CMAKE_SOURCE_DIR}/plasma-workspace/ksmserver/screenlocker/dbus/org.freedesktop.ScreenSaver.xml)
    set(krunner_xml ${CMAKE_SOURCE_DIR}/plasma-workspace/krunner/dbus/org.kde.krunner.App.xml)
    set(ksmserver_xml ${CMAKE_SOURCE_DIR}/plasma-workspace/ksmserver/org.kde.KSMServerInterface.xml)
endif()

qt5_add_dbus_interface(libkickoff_SRCS ${screensaver_xml} screensaver_interface)
qt5_add_dbus_interface(libkickoff_SRCS ${krunner_xml} krunner_interface)
qt5_add_dbus_interface(libkickoff_SRCS ${ksmserver_xml} ksmserver_interface)

install(FILES core/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kickoff)
add_library(kickoffplugin SHARED ${libkickoff_SRCS})
target_link_libraries(kickoffplugin
                              Qt5::Core
                              Qt5::Qml
                              Qt5::DBus
                              KF5::CoreAddons
                              KF5::I18n
                              KF5::KIOCore
                              KF5::KIOWidgets
                              KF5::KIOFileWidgets
                              KF5::Runner
                              KF5::Solid
                              KF5::Service
                              KF5::KDE4Support
                              PW::KWorkspace)
install(TARGETS kickoffplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kickoff)

