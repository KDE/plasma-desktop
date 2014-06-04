# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmworkspaceoptions\")
########### next target ###############

set(kcm_workspaceoptions_PART_SRCS workspaceoptions.cpp )
ki18n_wrap_ui(kcm_workspaceoptions_PART_SRCS mainpage.ui)

kde4_add_plugin(kcm_workspaceoptions ${kcm_workspaceoptions_PART_SRCS})

target_link_libraries(kcm_workspaceoptions ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kcm_workspaceoptions  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES workspaceoptions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
