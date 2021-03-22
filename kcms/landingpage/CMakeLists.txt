# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_landingpage\")

########### next target ###############
set(kcm_landingpage_SRCS
  landingpage.cpp
  landingpagedata.cpp
)

kcmutils_generate_module_data(
    kcm_landingpage_SRCS
    MODULE_DATA_HEADER landingpagedata.h
    MODULE_DATA_CLASS_NAME LandingPageData
    SETTINGS_HEADERS landingpage_kdeglobalssettings.h landingpage_baloosettings.h
    SETTINGS_CLASSES LandingPageGlobalsSettings BalooSettings
)

kconfig_add_kcfg_files(kcm_landingpage_SRCS landingpage_kdeglobalssettings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_landingpage_SRCS landingpage_baloosettings.kcfgc GENERATE_MOC)

add_library(kcm_landingpage MODULE ${kcm_landingpage_SRCS})

target_link_libraries(kcm_landingpage
    KF5::QuickAddons
    KF5::I18n
    KF5::KCMUtils
    KF5::ConfigWidgets
    KF5::Declarative
    KF5::KDELibs4Support # kglobalsettings
    PW::KWorkspace
    KF5::ActivitiesStats

    Qt5::DBus
)

if(X11_FOUND)
    target_link_libraries(kcm_landingpage ${X11_LIBRARIES} Qt5::X11Extras)
endif()

kcoreaddons_desktop_to_json(kcm_landingpage "kcm_landingpage.desktop" SERVICE_TYPES kcmodule.desktop)

########### install files ###############
install(FILES landingpage_kdeglobalssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES landingpage_baloosettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kcm_landingpage.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_landingpage DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_landingpage kcms)
