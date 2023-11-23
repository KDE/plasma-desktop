add_definitions(-DTRANSLATION_DOMAIN=\"plasmaactivitymanager\")

ecm_add_qml_module(activityswitcherextensionplugin URI org.kde.plasma.activityswitcher GENERATE_PLUGIN_SOURCE)

target_sources(activityswitcherextensionplugin PRIVATE
   switcherbackend.cpp
   sortedactivitiesmodel.cpp
)

target_link_libraries(activityswitcherextensionplugin PRIVATE
   Qt::Core
   Qt::Quick
   Qt::Qml
   Qt::Widgets

   KF6::I18n
   Plasma::Activities
   KF6::ConfigCore
   KF6::CoreAddons
   KF6::GlobalAccel
   KF6::GuiAddons
   KF6::KIOCore
   KF6::KIOWidgets
   KF6::WindowSystem

   PW::LibTaskManager
   )

ecm_finalize_qml_module(activityswitcherextensionplugin)
