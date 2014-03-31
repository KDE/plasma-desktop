


########### next target ###############

set(kcm_display_PART_SRCS display.cpp )


add_library(kcm_display MODULE ${kcm_display_PART_SRCS})


target_link_libraries(kcm_display
    KF5::KCMUtils
    KF5::I18n
    KF5::KDE4Support
)

install(TARGETS kcm_display  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES display.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
