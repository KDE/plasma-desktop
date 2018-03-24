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

set(kickerplugin_SRCS
    plugin/abstractentry.cpp
    plugin/abstractmodel.cpp
    plugin/actionlist.cpp
    plugin/appentry.cpp
    plugin/appsmodel.cpp
    plugin/computermodel.cpp
    plugin/contactentry.cpp
    plugin/containmentinterface.cpp
    plugin/draghelper.cpp
    plugin/simplefavoritesmodel.cpp
    plugin/kastatsfavoritesmodel.cpp
    plugin/fileentry.cpp
    plugin/forwardingmodel.cpp
    plugin/placeholdermodel.cpp
    plugin/funnelmodel.cpp
    plugin/dashboardwindow.cpp
    plugin/kickerplugin.cpp
    plugin/menuentryeditor.cpp
    plugin/processrunner.cpp
    plugin/rootmodel.cpp
    plugin/runnermodel.cpp
    plugin/runnermatchesmodel.cpp
    plugin/recentcontactsmodel.cpp
    plugin/recentusagemodel.cpp
    plugin/submenu.cpp
    plugin/systementry.cpp
    plugin/systemmodel.cpp
    plugin/systemsettings.cpp
    plugin/wheelinterceptor.cpp
    plugin/windowsystem.cpp
    plugin/funnelmodel.cpp
)

ecm_qt_declare_logging_category(kickerplugin_SRCS
    HEADER debug.h
    IDENTIFIER KICKER_DEBUG
    CATEGORY_NAME org.kde.plasma.kicker)

qt5_add_dbus_interface(kickerplugin_SRCS ${KRUNNERAPP_INTERFACE} krunner_interface)
qt5_add_dbus_interface(kickerplugin_SRCS ${KSMSERVER_DBUS_INTERFACE} ksmserver_interface)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kicker)

add_library(kickerplugin SHARED ${kickerplugin_SRCS})

target_link_libraries(kickerplugin
                      Qt5::Core
                      Qt5::DBus
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::X11Extras
                      KF5::Activities
                      KF5::ActivitiesStats
                      KF5::ConfigCore
                      KF5::CoreAddons
                      KF5::I18n
                      KF5::ItemModels
                      KF5::KDELibs4Support # FIXME: New Solid power management API doesn't exist yet, so we need to use deprecated stuff.
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::People
                      KF5::PeopleWidgets
                      KF5::PlasmaQuick
                      KF5::Runner
                      KF5::Service
                      KF5::Solid
                      KF5::WindowSystem
                      PW::KWorkspace)

if (${HAVE_APPSTREAMQT})
target_link_libraries(kickerplugin AppStreamQt)
endif()

install(TARGETS kickerplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kicker)
