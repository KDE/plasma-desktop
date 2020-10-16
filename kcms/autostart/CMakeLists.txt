# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_autostart\")

set(kcm_autostart_PART_SRCS
   autostartmodel.cpp
   autostart.cpp )

add_library(kcm_autostart MODULE ${kcm_autostart_PART_SRCS})

target_link_libraries(kcm_autostart KF5::I18n KF5::KIOCore KF5::KIOWidgets KF5::QuickAddons)

kcoreaddons_desktop_to_json(kcm_autostart "package/metadata.desktop")

install(TARGETS kcm_autostart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms )
install(FILES package/metadata.desktop RENAME autostart.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
kpackage_install_package(package kcm_autostart kcms)
