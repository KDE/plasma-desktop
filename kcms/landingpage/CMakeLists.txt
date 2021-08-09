# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_landingpage\")

set(HAVE_KUSERFEEDBACK ${KUserFeedback_FOUND})

configure_file(config-landingpage.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-landingpage.h)

########### next target ###############
set(kcm_landingpage_SRCS
  landingpage.cpp
  landingpagedata.cpp
)

if (KUserFeedback_FOUND)
set(FEEDBACKSETTINGS_HEADER landingpage_feedbacksettings.h)
set(FEEDBACKSETTINGS_SHORTHEADER FeedbackSettings)
endif()

kcmutils_generate_module_data(
    kcm_landingpage_SRCS
    MODULE_DATA_HEADER landingpagedata.h
    MODULE_DATA_CLASS_NAME LandingPageData
    SETTINGS_HEADERS landingpage_kdeglobalssettings.h ${FEEDBACKSETTINGS_HEADER}
    SETTINGS_CLASSES LandingPageGlobalsSettings ${FEEDBACKSETTINGS_SHORTHEADER}
)

kconfig_add_kcfg_files(kcm_landingpage_SRCS landingpage_kdeglobalssettings.kcfgc GENERATE_MOC)
if (KUserFeedback_FOUND)
kconfig_add_kcfg_files(kcm_landingpage_SRCS landingpage_feedbacksettings.kcfgc GENERATE_MOC)
endif()

kcoreaddons_add_plugin(kcm_landingpage SOURCES ${kcm_landingpage_SRCS} INSTALL_NAMESPACE "kcms")

target_link_libraries(kcm_landingpage
    KF5::QuickAddons
    KF5::I18n
    KF5::KCMUtils
    KF5::ConfigWidgets
    KF5::Declarative
    PW::KWorkspace
    KF5::ActivitiesStats

    Qt5::DBus
)

if(X11_FOUND)
    target_link_libraries(kcm_landingpage X11::X11 Qt5::X11Extras)
endif()

if (KUserFeedback_FOUND)
    target_link_libraries(kcm_landingpage KUserFeedbackCore)
endif()

kcoreaddons_desktop_to_json(kcm_landingpage "kcm_landingpage.desktop" SERVICE_TYPES kcmodule.desktop)

########### install files ###############
install(FILES landingpage_kdeglobalssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
if (KUserFeedback_FOUND)
install(FILES landingpage_feedbacksettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
endif()
install(FILES kcm_landingpage.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

kpackage_install_package(package kcm_landingpage kcms)
