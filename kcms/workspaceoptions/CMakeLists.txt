# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_workspace\")

########### next target ###############
set(kcm_workspace_SRCS
  workspaceoptions.cpp
  workspaceoptionsdata.cpp
)

kcmutils_generate_module_data(
    kcm_workspace_SRCS
    MODULE_DATA_HEADER workspaceoptionsdata.h
    MODULE_DATA_CLASS_NAME WorkspaceOptionsData
    SETTINGS_HEADERS workspaceoptions_plasmasettings.h workspaceoptions_kdeglobalssettings.h
    SETTINGS_CLASSES WorkspaceOptionsPlasmaSettings WorkspaceOptionsGlobalsSettings
)

kconfig_add_kcfg_files(kcm_workspace_SRCS workspaceoptions_kdeglobalssettings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_workspace_SRCS workspaceoptions_plasmasettings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_workspace SOURCES ${kcm_workspace_SRCS} INSTALL_NAMESPACE "kcms")

target_link_libraries(kcm_workspace
    KF5::QuickAddons
    KF5::I18n
    KF5::KCMUtils
    KF5::ConfigWidgets
    KF5::Declarative

    Qt::DBus
)

kcoreaddons_desktop_to_json(kcm_workspace "kcm_workspace.desktop" SERVICE_TYPES kcmodule.desktop)

########### install files ###############
install(FILES workspaceoptions_kdeglobalssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES workspaceoptions_plasmasettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kcm_workspace.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

kpackage_install_package(package kcm_workspace kcms)
