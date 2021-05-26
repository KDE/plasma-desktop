set (
   activityswitcher_imports_LIB_SRCS
   activityswitcherextensionplugin.cpp
   switcherbackend.cpp
   sortedactivitiesmodel.cpp
   )

add_library (activityswitcherextensionplugin SHARED ${activityswitcher_imports_LIB_SRCS})
add_definitions(-DTRANSLATION_DOMAIN=\"plasmaactivitymanager\")

target_link_libraries (
   activityswitcherextensionplugin

   Qt::Core
   Qt::Quick
   Qt::Qml
   Qt::Widgets

   KF5::I18n
   KF5::Activities
   KF5::ConfigCore
   KF5::CoreAddons
   KF5::GlobalAccel
   KF5::GuiAddons
   KF5::KIOCore
   KF5::KIOWidgets
   KF5::WindowSystem

   PW::LibTaskManager
   )

## install

install (TARGETS activityswitcherextensionplugin
         DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/activityswitcher)
install (FILES qmldir
         DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/activityswitcher)

