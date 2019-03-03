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

ki18n_wrap_ui(kcm_componentchooser_SRCS
  browserconfig_ui.ui
  filemanagerconfig_ui.ui
  emailclientconfig_ui.ui
  componentchooser_ui.ui
  componentconfig_ui.ui
  terminalemulatorconfig_ui.ui
)

add_library(kcm_componentchooser MODULE ${kcm_componentchooser_SRCS})



target_link_libraries(kcm_componentchooser
                      KF5::KDELibs4Support
                      Qt5::X11Extras
                      )

install(TARGETS kcm_componentchooser  DESTINATION ${KDE_INSTALL_PLUGINDIR} )


########### install files ###############

install( FILES componentchooser.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )

