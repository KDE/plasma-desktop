# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmstyle\")

########### next target ###############

set(kcm_style_PART_SRCS ../krdb/krdb.cpp styleconfdialog.cpp kcmstyle.cpp)
set(klauncher_xml ${KINIT_DBUS_INTERFACES_DIR}/kf5_org.kde.KLauncher.xml)
qt5_add_dbus_interface(kcm_style_PART_SRCS ${klauncher_xml} klauncher_iface)


ki18n_wrap_ui(kcm_style_PART_SRCS stylepreview.ui styleconfig.ui)

add_library(kcm_style MODULE ${kcm_style_PART_SRCS})

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
    KF5::NewStuff
)

install(TARGETS kcm_style  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES style.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
