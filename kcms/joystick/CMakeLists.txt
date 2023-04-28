# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_joystick\")

kcoreaddons_add_plugin(kcm_joystick INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
kcmutils_generate_desktop_file(kcm_joystick)

target_sources(kcm_joystick PRIVATE
   joystick.cpp 
   joywidget.cpp 
   poswidget.cpp 
   joydevice.cpp 
   caldialog.cpp
)

target_link_libraries(kcm_joystick
    KF6::Completion
    KF6::KCMUtils
    KF6::I18n
    KF6::KIOWidgets
)
