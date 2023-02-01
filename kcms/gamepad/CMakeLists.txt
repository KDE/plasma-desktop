# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_gamepad\")

set(kcm_gamepad_SRCS
  kcm.cpp
  devicemodel.cpp
  joydevice.cpp
)

kcoreaddons_add_plugin(kcm_gamepad SOURCES ${kcm_gamepad_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_gamepad)

target_link_libraries(kcm_gamepad
  KF5::CoreAddons
  KF5::ConfigWidgets
  KF5::KCMUtils
  KF5::QuickAddons
  KF5::I18n
  KF5::Plasma
  KF5::PlasmaQuick
  KF5::Solid

  Qt::Widgets
  Qt::QuickWidgets

  udev
  evdev
)

kpackage_install_package(package kcm_gamepad kcms)
