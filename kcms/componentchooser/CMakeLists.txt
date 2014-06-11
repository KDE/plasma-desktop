# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmcomponentchooser\")

add_subdirectory( componentservices ) 
add_subdirectory( windowmanagers ) 



########### next target ###############

set(kcm_componentchooser_SRCS
  componentchooser.cpp
  componentchooserbrowser.cpp
  componentchooserfilemanager.cpp
  componentchooseremail.cpp
  kcm_componentchooser.cpp
)

if(NOT WIN32)
  set(kcm_componentchooser_SRCS ${kcm_componentchooser_SRCS}
    componentchooserterminal.cpp ktimerdialog.cpp)
endif()

if(X11_FOUND)
  set(kcm_componentchooser_SRCS ${kcm_componentchooser_SRCS}
     componentchooserwm.cpp )
endif()

ki18n_wrap_ui(kcm_componentchooser_SRCS
  browserconfig_ui.ui
  filemanagerconfig_ui.ui
  emailclientconfig_ui.ui
  componentchooser_ui.ui
  componentconfig_ui.ui
)

if(NOT WIN32)
  ki18n_wrap_ui(kcm_componentchooser_SRCS terminalemulatorconfig_ui.ui wmconfig_ui.ui)
endif()

add_library(kcm_componentchooser MODULE ${kcm_componentchooser_SRCS})



target_link_libraries(kcm_componentchooser
                      KF5::KDELibs4Support
                      )
if(X11_FOUND)
  target_link_libraries(kcm_componentchooser Qt5::X11Extras)
endif()

install(TARGETS kcm_componentchooser  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES componentchooser.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

