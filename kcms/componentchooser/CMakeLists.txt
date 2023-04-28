# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_componentchooser\")

set(componentchooser_SRCS
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

kconfig_add_kcfg_files(componentchooser_SRCS browser_settings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(componentchooser_SRCS terminal_settings.kcfgc GENERATE_MOC)

kcmutils_add_qml_kcm(kcm_componentchooser SOURCES ${componentchooser_SRCS})

target_link_libraries(kcm_componentchooser
    Qt::Core
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    KF6::Service
    KF6::KIOWidgets
    KF6::ConfigGui
    KF6::KCMUtils
)

kpackage_install_package(package kcm_componentchooser kcms)
