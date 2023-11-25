# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_smserver\")

kcmutils_add_qml_kcm(kcm_smserver)

kcmutils_generate_module_data(kcm_smserver
    MODULE_DATA_HEADER smserverdata.h
    MODULE_DATA_CLASS_NAME SMServerData
    SETTINGS_HEADERS smserversettings.h
    SETTINGS_CLASSES SMServerSettings
)
kconfig_add_kcfg_files(kcm_smserver smserversettings.kcfgc GENERATE_MOC)
qt_add_dbus_interface(kcm_smserver_DBUS_SRCS org.freedesktop.login1.Manager.xml login1_manager)

target_sources(kcm_smserver PRIVATE kcmsmserver.cpp ${kcm_smserver_DBUS_SRCS})

target_link_libraries(kcm_smserver PRIVATE
    Qt::Core
    Qt::DBus
    KF6::KCMUtilsQuick
    KF6::I18n
    KF6::ConfigCore
    KF6::KCMUtils
    PW::KWorkspace)

