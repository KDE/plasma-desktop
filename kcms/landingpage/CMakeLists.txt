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
    SETTINGS_HEADERS landingpage_kdeglobalssettings.h
    SETTINGS_CLASSES LandingPageGlobalsSettings
)

kconfig_add_kcfg_files(kcm_landingpage_SRCS landingpage_kdeglobalssettings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_landingpage SOURCES ${kcm_landingpage_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_landingpage)

target_link_libraries(kcm_landingpage
    KF5::QuickAddons
    KF5::I18n
    KF5::KCMUtils
    KF5::ConfigWidgets
    KF5::Declarative
    KF5::Package
    PW::KWorkspace
    KF5::ActivitiesStats

    Qt::DBus
)

if(X11_FOUND)
    target_link_libraries(kcm_landingpage X11::X11)
    if (QT_MAJOR_VERSION EQUAL "5")
        target_link_libraries(kcm_landingpage Qt5::X11Extras)
    endif()
endif()

########### install files ###############
install(FILES landingpage_kdeglobalssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

kpackage_install_package(package kcm_landingpage kcms)
