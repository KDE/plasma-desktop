add_definitions(-DTRANSLATION_DOMAIN=\"kcm_activities\")

kcmutils_add_qml_kcm(kcm_activities
   SOURCES
      kcm_activities.cpp
      activityconfig.cpp
)

target_link_libraries (kcm_activities PRIVATE
   Qt::Quick
   Qt::Core
   Qt::Qml
   Qt::DBus
   KF6::CoreAddons
   KF6::KCMUtils
   KF6::I18n
   KF6::Service
   KF6::XmlGui
   KF6::GlobalAccel
   Plasma::Activities
)
