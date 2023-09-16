# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_workspace\")

kcmutils_add_qml_kcm(kcm_workspace SOURCES workspaceoptions.cpp workspaceoptionsdata.cpp)

kcmutils_generate_module_data(kcm_workspace
    MODULE_DATA_HEADER workspaceoptionsdata.h
    MODULE_DATA_CLASS_NAME WorkspaceOptionsData
    SETTINGS_HEADERS workspaceoptions_plasmasettings.h workspaceoptions_kdeglobalssettings.h workspaceoptions_kwinsettings.h
    SETTINGS_CLASSES WorkspaceOptionsPlasmaSettings WorkspaceOptionsGlobalsSettings WorkspaceOptionsKwinSettings
)

kconfig_add_kcfg_files(kcm_workspace workspaceoptions_kdeglobalssettings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_workspace workspaceoptions_plasmasettings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_workspace workspaceoptions_kwinsettings.kcfgc GENERATE_MOC)

target_link_libraries(kcm_workspace PRIVATE
    KF6::KCMUtilsQuick
    KF6::I18n
    KF6::KCMUtils
    KF6::ConfigWidgets

    Qt::DBus
)

########### install files ###############
install(FILES workspaceoptions_kdeglobalssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES workspaceoptions_plasmasettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES workspaceoptions_kwinsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

