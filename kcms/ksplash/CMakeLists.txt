# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_splashscreen\")

set(kcm_splashscreen_SRCS
  kcm.cpp
)

kcmutils_generate_module_data(
    kcm_splashscreen_SRCS
    MODULE_DATA_HEADER splashscreendata.h
    MODULE_DATA_CLASS_NAME SplashScreenData
    SETTINGS_HEADERS splashscreensettings.h
    SETTINGS_CLASSES SplashScreenSettings
)

kconfig_add_kcfg_files(kcm_splashscreen_SRCS splashscreensettings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_splashscreen SOURCES ${kcm_splashscreen_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_splashscreen)

target_link_libraries(kcm_splashscreen
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::ConfigWidgets
  KF5::KCMUtils
  KF5::QuickAddons
  KF5::I18n
  KF5::Plasma
  KF5::PlasmaQuick
  KF5::NewStuff
  KF5::Package

  Qt::DBus
  Qt::Widgets
  Qt::QuickWidgets
)

#this desktop file is installed only for retrocompatibility with sycoca
install(FILES splashscreensettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES ksplash.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR} )

kpackage_install_package(package kcm_splashscreen kcms)
