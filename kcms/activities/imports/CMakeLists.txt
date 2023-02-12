# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-settings)
find_package (Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS Sql Gui Widgets Qml Quick Sql QuickWidgets)
find_package (KF6GlobalAccel ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6I18n       ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6WidgetsAddons ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6Config     ${KF6_MIN_VERSION} CONFIG REQUIRED)

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
   KF6::I18n
   KF6::Activities
   KF6::ConfigCore
   KF6::GlobalAccel
   KF6::WidgetsAddons
   )

## install

install (
   DIRECTORY
   qml/activityDialog

   DESTINATION ${KAMD_KCM_DATADIR}/qml
   )

install (TARGETS kactivitiessettingsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities/settings)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities/settings)

