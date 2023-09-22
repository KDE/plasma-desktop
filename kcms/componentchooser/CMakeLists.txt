# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_componentchooser\")

kcmutils_add_qml_kcm(kcm_componentchooser)

target_sources(kcm_componentchooser PRIVATE
    kcm_componentchooser.cpp
    componentchooser.cpp
    applicationmodel.cpp
    components/componentchooserbrowser.cpp
    components/componentchooseremail.cpp
    components/componentchooserterminal.cpp
    components/componentchooserfilemanager.cpp
    components/componentchoosertexteditor.cpp
    components/componentchooserimageviewer.cpp
    components/componentchoosermusicplayer.cpp
    components/componentchooservideoplayer.cpp
    components/componentchooserpdfviewer.cpp
    components/componentchooserarchivemanager.cpp
    components/componentchoosergeo.cpp
    components/componentchoosertel.cpp
)

kconfig_add_kcfg_files(kcm_componentchooser terminal_settings.kcfgc GENERATE_MOC)

target_link_libraries(kcm_componentchooser PRIVATE
    Qt::Core
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    KF6::Service
    KF6::KIOWidgets
    KF6::ConfigGui
    KF6::KCMUtils
)

