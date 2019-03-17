# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_splashscreen\")

set(kcm_splashscreen_SRCS
  kcm.cpp
)

add_library(kcm_splashscreen MODULE ${kcm_splashscreen_SRCS})

target_link_libraries(kcm_splashscreen
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::QuickAddons
  KF5::I18n
  KF5::Plasma
  KF5::PlasmaQuick
  KF5::NewStuff

  Qt5::DBus
  Qt5::Widgets
  Qt5::QuickWidgets
)

kcoreaddons_desktop_to_json(kcm_splashscreen "kcm_splashscreen.desktop" SERVICE_TYPES kcmodule.desktop)

#this desktop file is installed only for retrocompatibility with sycoca
install(FILES kcm_splashscreen.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_splashscreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install( FILES ksplash.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR} )

kpackage_install_package(package kcm_splashscreen kcms)
