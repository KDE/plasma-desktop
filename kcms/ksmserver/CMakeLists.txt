# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmsmserver\")

set(kcm_smserver_PART_SRCS kcmsmserver.cpp smserverconfigimpl.cpp )


ki18n_wrap_ui(kcm_smserver_PART_SRCS smserverconfigdlg.ui )

add_library(kcm_smserver MODULE ${kcm_smserver_PART_SRCS})


target_link_libraries(kcm_smserver Qt5::Core Qt5::DBus Qt5::Widgets KF5::I18n KF5::ConfigCore KF5::KCMUtils PW::KWorkspace)

install(TARGETS kcm_smserver  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmsmserver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

