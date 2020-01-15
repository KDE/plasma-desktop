# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_desktoptheme\")

set(kcm_desktoptheme_SRCS
  kcm.cpp
  themesmodel.cpp
  filterproxymodel.cpp
)

kconfig_add_kcfg_files(kcm_desktoptheme_SRCS desktopthemesettings.kcfgc GENERATE_MOC)

add_library(kcm_desktoptheme MODULE ${kcm_desktoptheme_SRCS})

target_link_libraries(kcm_desktoptheme
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::KIOCore
  KF5::KIOWidgets
  KF5::I18n
  KF5::Plasma
  KF5::Declarative
  KF5::QuickAddons
)

kcoreaddons_desktop_to_json(kcm_desktoptheme "kcm_desktoptheme.desktop" SERVICE_TYPES kcmodule.desktop)

#this desktop file is installed only for retrocompatibility with sycoca
install(FILES kcm_desktoptheme.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES plasma-themes.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(TARGETS kcm_desktoptheme DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_desktoptheme kcms)
