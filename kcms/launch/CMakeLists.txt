# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_launchfeedback\")

########### next target ###############

set(kcm_launchfeedback_PART_SRCS launchfeedback.cpp)

set_source_files_properties(${KWIN_EFFECTS_INTERFACE} PROPERTIES INCLUDE "interface_util.h")
qt5_add_dbus_interface(kcm_launchfeedback_PART_SRCS ${KWIN_EFFECTS_INTERFACE} kwin_interface)

add_library(kcm_launchfeedback MODULE ${kcm_launchfeedback_PART_SRCS})

target_link_libraries(kcm_launchfeedback
    Qt5::DBus
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
)

kcoreaddons_desktop_to_json(kcm_launchfeedback "kcm_launchfeedback.desktop")

########### install files ###############

install(TARGETS kcm_launchfeedback DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install(FILES kcm_launchfeedback.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
kpackage_install_package(package kcm_launchfeedback kcms)

