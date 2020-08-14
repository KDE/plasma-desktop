# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_style\")

########### next target ###############

set(kcm_style_PART_SRCS
    ../krdb/krdb.cpp
    styleconfdialog.cpp
    kcmstyle.cpp
    stylesmodel.cpp
    gtkthemesmodel.cpp
    gtkpage.cpp
    previewitem.cpp
    styledata.cpp
)

ki18n_wrap_ui(kcm_style_PART_SRCS stylepreview.ui)

kconfig_add_kcfg_files(kcm_style_PART_SRCS stylesettings.kcfgc GENERATE_MOC)

add_library(kcm_style MODULE ${kcm_style_PART_SRCS})

target_compile_definitions(kcm_style PUBLIC CMAKE_INSTALL_FULL_LIBEXECDIR="${CMAKE_INSTALL_FULL_LIBEXECDIR}")

target_link_libraries(kcm_style
    Qt5::X11Extras
    Qt5::DBus
    KF5::KCMUtils
    KF5::Completion
    KF5::I18n
    KF5::Notifications
    KF5::Plasma
    ${X11_LIBRARIES}
    KF5::KDELibs4Support
    KF5::GuiAddons
    KF5::QuickAddons
    KF5::WindowSystem
    KF5::Archive
    KF5::NewStuff
    PW::KWorkspace
)

kcoreaddons_desktop_to_json(kcm_style "kcm_style.desktop")

install(FILES stylesettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES style_widgetstyle_default_breeze.upd style_widgetstyle_default_breeze.pl DESTINATION ${KDE_INSTALL_DATADIR}/kconf_update)
install(FILES gtk_themes.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(FILES kcm_style.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_style DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_style kcms)
