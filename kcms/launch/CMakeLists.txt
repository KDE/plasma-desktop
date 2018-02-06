# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"kcmlaunchfeedback\")

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

install(TARGETS kcm_launchfeedback DESTINATION ${PLUGIN_INSTALL_DIR}/kcms)
install(FILES kcm_launchfeedback.desktop  DESTINATION  ${SERVICES_INSTALL_DIR})
kpackage_install_package(package kcm_launchfeedback kcms)

