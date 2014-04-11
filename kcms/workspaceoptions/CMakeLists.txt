########### next target ###############

set(kcm_workspaceoptions_PART_SRCS workspaceoptions.cpp )
kde4_add_ui_files(kcm_workspaceoptions_PART_SRCS mainpage.ui)

kde4_add_plugin(kcm_workspaceoptions ${kcm_workspaceoptions_PART_SRCS})

target_link_libraries(kcm_workspaceoptions ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kcm_workspaceoptions  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES workspaceoptions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
