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

   KF6::I18n
   KF6::Activities
   KF6::ConfigCore
   KF6::CoreAddons
   KF6::GlobalAccel
   KF6::GuiAddons
   KF6::KIOCore
   KF6::KIOWidgets
   KF6::WindowSystem

   PW::LibTaskManager
   )

## install

install (TARGETS activityswitcherextensionplugin
         DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/activityswitcher)
install (FILES qmldir
         DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/activityswitcher)

