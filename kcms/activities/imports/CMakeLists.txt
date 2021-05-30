# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-settings)
find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql Gui Widgets Qml Quick Sql QuickWidgets)
find_package (KF5GlobalAccel ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5I18n       ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5WidgetsAddons ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5Config     ${KF5_DEP_VERSION} CONFIG REQUIRED)

set (
   kactivities_KCM_imports_LIB_SRCS
   plugin.cpp
   activitysettings.cpp
   dialog.cpp
   )

qt_add_dbus_interface (
   kactivities_KCM_imports_LIB_SRCS

   ../common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
   )

add_library (kactivitiessettingsplugin SHARED ${kactivities_KCM_imports_LIB_SRCS})

target_link_libraries (
   kactivitiessettingsplugin
   Qt::Core
   Qt::DBus
   Qt::Gui
   Qt::Qml
   Qt::Quick
   Qt::Widgets
   Qt::QuickWidgets
   KF5::I18n
   KF5::Activities
   KF5::ConfigCore
   KF5::GlobalAccel
   KF5::WidgetsAddons
   )

## install

install (
   DIRECTORY
   qml/activityDialog

   DESTINATION ${KAMD_KCM_DATADIR}/qml
   )

install (TARGETS kactivitiessettingsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities/settings)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities/settings)

