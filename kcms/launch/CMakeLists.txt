# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_launchfeedback\")

########### next target ###############

set(kcm_launchfeedback_PART_SRCS launchfeedback.cpp launchfeedbacksettings.cpp)

kcmutils_generate_module_data(
    kcm_launchfeedback_PART_SRCS
    MODULE_DATA_HEADER launchfeedbackdata.h
    MODULE_DATA_CLASS_NAME LaunchFeedbackData
    SETTINGS_HEADERS launchfeedbacksettings.h
    SETTINGS_CLASSES LaunchFeedbackSettings
)

kconfig_add_kcfg_files(kcm_launchfeedback_PART_SRCS launchfeedbacksettingsbase.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_launchfeedback SOURCES ${kcm_launchfeedback_PART_SRCS} INSTALL_NAMESPACE "kcms")

target_link_libraries(kcm_launchfeedback
    Qt::DBus
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
)

kcoreaddons_desktop_to_json(kcm_launchfeedback "kcm_launchfeedback.desktop")

########### install files ###############

install(FILES launchfeedbacksettingsbase.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kcm_launchfeedback.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
kpackage_install_package(package kcm_launchfeedback kcms)
