# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_emoticons\")

set(kcmemoticons_SRCS emoticonslist.cpp)
ki18n_wrap_ui(kcmemoticons_SRCS emoticonslist.ui)

add_library(kcm_emoticons MODULE ${kcmemoticons_SRCS})
target_link_libraries(kcm_emoticons
                      KF5::NewStuff
                      KF5::KDELibs4Support
                      KF5::Emoticons)

install(TARGETS kcm_emoticons  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES emoticons.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES emoticons.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
