# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmcolors\")

set(kcm_colors_SRCS ../krdb/krdb.cpp colorscm.cpp previewwidget.cpp setpreviewwidget.cpp)

set(klauncher_xml ${KINIT_DBUS_INTERFACES_DIR}/kf5_org.kde.KLauncher.xml)
qt5_add_dbus_interface(kcm_colors_SRCS ${klauncher_xml} klauncher_iface)

ki18n_wrap_ui(kcm_colors_SRCS colorsettings.ui preview.ui setpreview.ui)

add_library(kcm_colors MODULE ${kcm_colors_SRCS})
target_link_libraries(kcm_colors
    KF5::KCMUtils
    KF5::GuiAddons
    KF5::I18n
    Qt5::DBus
    KF5::NewStuff
    KF5::KDELibs4Support
    ${X11_LIBRARIES})

if(X11_FOUND)
    target_link_libraries(kcm_colors Qt5::X11Extras)
endif()

install(TARGETS kcm_colors  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES colors.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES colorschemes.knsrc DESTINATION  ${CONFIG_INSTALL_DIR} )

# built-in color schemes
file(GLOB schemefiles schemes/*.colors)
install( FILES ${schemefiles} DESTINATION ${DATA_INSTALL_DIR}/color-schemes )
