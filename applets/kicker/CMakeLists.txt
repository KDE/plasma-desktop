add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
# -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.kicker\"
)

plasma_install_package(package org.kde.plasma.kicker)

include_directories(plugin/plasmaquick)

set(kickerplugin_SRCS
    plugin/abstractentry.cpp
    plugin/abstractmodel.cpp
    plugin/actionlist.cpp
    plugin/appsmodel.cpp
    plugin/containmentinterface.cpp
    plugin/draghelper.cpp
    plugin/favoritesmodel.cpp
    plugin/funnelmodel.cpp
    plugin/imagepicker.cpp
    plugin/kickerplugin.cpp
    plugin/menuentryeditor.cpp
    plugin/processrunner.cpp
    plugin/rootmodel.cpp
    plugin/runnermodel.cpp
    plugin/runnermatchesmodel.cpp
    plugin/recentappsmodel.cpp
    plugin/recentdocsmodel.cpp
    plugin/submenu.cpp
    plugin/systemmodel.cpp
    plugin/windowsystem.cpp
)

if(PackageKitQt5_FOUND)
    list(APPEND kickerplugin_SRCS plugin/findpackagenamejob.cpp)
endif()

qt5_add_dbus_interface(kickerplugin_SRCS ${KSMSERVER_DBUS_INTERFACE} ksmserver_interface)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kicker)

add_library(kickerplugin SHARED ${kickerplugin_SRCS})

target_link_libraries(kickerplugin
                      Qt5::Core
                      Qt5::DBus
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets # FIXME: Used for QFileDialog; can be removed when switching to Qt Quick Dialogs (see bug 334963).
                      Qt5::X11Extras
                      KF5::ConfigCore
                      KF5::CoreAddons
                      KF5::I18n
                      KF5::KDELibs4Support # FIXME: New Solid power management API doesn't exist yet, so we need to use deprecated stuff.
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::PlasmaQuick
                      KF5::Runner
                      KF5::Service
                      KF5::Solid
                      KF5::WindowSystem
                      PW::KWorkspace)

if(PackageKitQt5_FOUND)
    target_link_libraries(kickerplugin PK::packagekitqt5)
endif()

install(TARGETS kickerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kicker)
