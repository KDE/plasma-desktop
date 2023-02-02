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

kcoreaddons_add_plugin(kcm_componentchooser SOURCES ${componentchooser_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_componentchooser)

target_link_libraries(kcm_componentchooser
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::QuickAddons
    KF5::Service
    KF5::KIOWidgets
    KF5::ConfigGui
    KF5::KCMUtils
)

kpackage_install_package(package kcm_componentchooser kcms)
