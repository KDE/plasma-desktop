# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_componentchooser\")

add_subdirectory( componentservices ) 



########### next target ###############

set(kcm_componentchooser_SRCS
  componentchooser.cpp
  componentchooserbrowser.cpp
  componentchooserfilemanager.cpp
  componentchooseremail.cpp
  componentchooserterminal.cpp
  kcm_componentchooser.cpp
)
kconfig_add_kcfg_files(kcm_componentchooser_SRCS browser_settings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_componentchooser_SRCS terminal_settings.kcfgc GENERATE_MOC)

ki18n_wrap_ui(kcm_componentchooser_SRCS
  componentchooser_ui.ui
)

add_library(kcm_componentchooser MODULE ${kcm_componentchooser_SRCS})



target_link_libraries(kcm_componentchooser
                      KF5::I18n
                      KF5::Completion
                      KF5::ConfigWidgets
                      KF5::Service
                      KF5::KIOWidgets
                      KF5::KDELibs4Support
                      Qt5::DBus
                      Qt5::X11Extras
                      )

install(TARGETS kcm_componentchooser  DESTINATION ${KDE_INSTALL_PLUGINDIR} )


########### install files ###############

install( FILES componentchooser.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
install(FILES browser_settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES terminal_settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

