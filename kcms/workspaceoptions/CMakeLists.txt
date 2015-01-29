# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmworkspaceoptions\")
########### next target ###############

set(kcm_workspaceoptions_PART_SRCS workspaceoptions.cpp )
ki18n_wrap_ui(kcm_workspaceoptions_PART_SRCS mainpage.ui)

add_library(kcm_workspaceoptions MODULE ${kcm_workspaceoptions_PART_SRCS})

target_link_libraries(kcm_workspaceoptions  KF5::KIOCore KF5::ConfigWidgets KF5::I18n)

install(TARGETS kcm_workspaceoptions  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES workspaceoptions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
