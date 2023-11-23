# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_landingpage\")

kcmutils_add_qml_kcm(kcm_landingpage SOURCES landingpage.cpp landingpagedata.cpp)

kcmutils_generate_module_data(kcm_landingpage
    MODULE_DATA_HEADER landingpagedata.h
    MODULE_DATA_CLASS_NAME LandingPageData
    SETTINGS_HEADERS landingpage_kdeglobalssettings.h
    SETTINGS_CLASSES LandingPageGlobalsSettings
)
kconfig_add_kcfg_files(kcm_landingpage landingpage_kdeglobalssettings.kcfgc GENERATE_MOC)

target_link_libraries(kcm_landingpage PRIVATE
    Qt::Quick
    Qt::DBus
    KF6::I18n
    KF6::KCMUtilsQuick
    KF6::KCMUtils
    KF6::ConfigWidgets
    KF6::Package
    PW::KWorkspace
    Plasma::ActivitiesStats
    KF6::Service
)

if(X11_FOUND)
    target_link_libraries(kcm_landingpage PRIVATE X11::X11)
endif()

install(FILES landingpage_kdeglobalssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
