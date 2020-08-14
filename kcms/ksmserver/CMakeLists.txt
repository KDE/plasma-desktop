# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmsmserver\")

set(kcm_smserver_PART_SRCS kcmsmserver.cpp smserverdata.cpp)

qt5_add_dbus_interface(kcm_smserver_PART_SRCS org.freedesktop.login1.Manager.xml login1_manager)

kconfig_add_kcfg_files(kcm_smserver_PART_SRCS smserversettings.kcfgc GENERATE_MOC)
ki18n_wrap_ui(kcm_smserver_PART_SRCS smserverconfigdlg.ui )

add_library(kcm_smserver MODULE ${kcm_smserver_PART_SRCS})


target_link_libraries(kcm_smserver Qt5::Core Qt5::DBus Qt5::Widgets KF5::I18n KF5::ConfigCore KF5::KCMUtils PW::KWorkspace)

install(TARGETS kcm_smserver  DESTINATION ${KDE_INSTALL_PLUGINDIR} )


########### install files ###############

install( FILES kcmsmserver.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )

add_subdirectory(kconf_update)
