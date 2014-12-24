# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmnotify\")

########### next target ###############

set(kcm_knotify_PART_SRCS knotify.cpp )


add_library(kcm_knotify MODULE ${kcm_knotify_PART_SRCS})


target_link_libraries(kcm_knotify
                        KF5::NotifyConfig
                        KF5::KDELibs4Support
                        )

install(TARGETS kcm_knotify  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmnotify.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


