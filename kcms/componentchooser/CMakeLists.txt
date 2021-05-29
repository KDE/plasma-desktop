# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_componentchooser\")

set(componentchooser_SRCS
    kcm_componentchooser.cpp
    componentchooser.cpp
    componentchooserbrowser.cpp
    componentchooseremail.cpp
    componentchooserterminal.cpp
    componentchooserfilemanager.cpp
    componentchooserdata.cpp
)

kconfig_add_kcfg_files(componentchooser_SRCS browser_settings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(componentchooser_SRCS terminal_settings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_componentchooser SOURCES ${componentchooser_SRCS} INSTALL_NAMESPACE "kcms")

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

kcoreaddons_desktop_to_json(kcm_componentchooser "package/metadata.desktop")

install(FILES package/metadata.desktop RENAME componentchooser.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
kpackage_install_package(package kcm_componentchooser kcms)
