# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_fonts\")

if(FONTCONFIG_FOUND)
    include_directories(${FONTCONFIG_INCLUDE_DIR})
endif()
include_directories(${FREETYPE_INCLUDE_DIRS})

########### next target ###############

include_directories(../kfontinst/lib)

set(kcm_fonts_PART_SRCS ../krdb/krdb.cpp previewrenderengine.cpp previewimageprovider.cpp fonts.cpp fontsaasettings.cpp fontsdata.cpp ../kfontinst/lib/FcEngine.cpp)

if(X11_FOUND)
    set(kcm_fonts_PART_SRCS ${kcm_fonts_PART_SRCS} ${libkxftconfig_SRCS})
endif()

kconfig_add_kcfg_files(kcm_fonts_PART_SRCS fontssettings.kcfgc fontsaasettingsbase.kcfgc GENERATE_MOC)

add_library(kcm_fonts MODULE ${kcm_fonts_PART_SRCS})

target_link_libraries(kcm_fonts Qt5::DBus Qt5::Xml KF5::KCMUtils KF5::I18n KF5::KDELibs4Support ${FREETYPE_LIBRARIES} kfontinst)

if(X11_FOUND)
    target_link_libraries(kcm_fonts
        Qt5::X11Extras
        KF5::I18n
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::Declarative
        KF5::QuickAddons
        KF5::WidgetsAddons
        PW::KWorkspace)
    if(FONTCONFIG_FOUND)
        target_link_libraries(kcm_fonts ${FONTCONFIG_LIBRARIES})
    endif()

    target_link_libraries(kcm_fonts ${X11_LIBRARIES} ${X11_Xft_LIB})
endif()

kcoreaddons_desktop_to_json(kcm_fonts "kcm_fonts.desktop")

########### install files ###############
install(FILES fontssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(TARGETS kcm_fonts DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install(FILES kcm_fonts.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
kpackage_install_package(package kcm_fonts kcms)
