# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_splashscreen\")

kcmutils_add_qml_kcm(kcm_splashscreen SOURCES kcm.cpp)

kcmutils_generate_module_data(kcm_splashscreen
    MODULE_DATA_HEADER splashscreendata.h
    MODULE_DATA_CLASS_NAME SplashScreenData
    SETTINGS_HEADERS splashscreensettings.h
    SETTINGS_CLASSES SplashScreenSettings
)
kconfig_add_kcfg_files(kcm_splashscreen splashscreensettings.kcfgc GENERATE_MOC)


target_link_libraries(kcm_splashscreen PRIVATE
  KF6::KIOWidgets
  KF6::CoreAddons
  KF6::ConfigWidgets
  KF6::KCMUtils
  KF6::KCMUtilsQuick
  KF6::I18n
  Plasma::Plasma
  Plasma::PlasmaQuick
  KF6::NewStuffCore
  KF6::Package

  Qt::DBus
  Qt::Widgets
  Qt::QuickWidgets
)

install(FILES splashscreensettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES ksplash.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR} )
