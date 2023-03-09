# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_desktoppaths\")

set(kcm_desktoppaths_PART_SRCS
    globalpaths.cpp
    desktoppathssettings.cpp
)

kcmutils_generate_module_data(
    kcm_desktoppaths_PART_SRCS
    MODULE_DATA_HEADER desktoppathsdata.h
    MODULE_DATA_CLASS_NAME DesktopPathsData
    SETTINGS_HEADERS desktoppathssettings.h
    SETTINGS_CLASSES DesktopPathsSettings
)

kcoreaddons_add_plugin(kcm_desktoppaths SOURCES ${kcm_desktoppaths_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_desktoppaths)

target_link_libraries(kcm_desktoppaths
    KF6::ConfigCore
    KF6::KCMUtilsQuick
    KF6::I18n
    KF6::KCMUtils
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::KIOFileWidgets
)

kpackage_install_package(package kcm_desktoppaths kcms)