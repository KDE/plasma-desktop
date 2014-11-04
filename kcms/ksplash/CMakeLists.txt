# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_splashscreen\")

set(kcm_splashscreen_SRCS
  kcm.cpp
)

add_library(kcm_splashscreen MODULE ${kcm_splashscreen_SRCS})

target_link_libraries(kcm_splashscreen
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::I18n
  KF5::Plasma
  KF5::PlasmaQuick

  Qt5::DBus
  Qt5::Widgets
  Qt5::QuickWidgets
)


install(FILES kcm_splashscreen.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kcm_splashscreen DESTINATION ${PLUGIN_INSTALL_DIR})

plasma_install_package(package kcm_splashscreen kcms kcm_splashscreen)
