project(desktoptheme)
# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_desktopthemedetails\")


set(kcmdesktoptheme_SRCS kcmdesktoptheme.cpp desktopthemedetails.cpp thememodel.cpp)
ki18n_wrap_ui(kcmdesktoptheme_SRCS DesktopTheme.ui DesktopThemeDetails.ui)

add_library(kcm_desktoptheme MODULE ${kcmdesktoptheme_SRCS})
target_link_libraries(kcm_desktoptheme ${X11_LIBRARIES}
    KF5::Archive
    KF5::KCMUtils
    KF5::I18n
    KF5::Parts
    KF5::Plasma
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::NewStuff
    KF5::KDELibs4Support
)

kcoreaddons_desktop_to_json(kcm_desktoptheme desktoptheme.desktop)

install(TARGETS kcm_desktoptheme DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES plasma-themes.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

########### install files ###############
install( FILES desktoptheme.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

