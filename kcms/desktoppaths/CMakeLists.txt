# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_desktoppaths\")

set(kcm_desktoppaths_PART_SRCS
   globalpaths.cpp
)

add_library(kcm_desktoppaths MODULE ${kcm_desktoppaths_PART_SRCS})

target_link_libraries(kcm_desktoppaths
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDELibs4Support
)

install(TARGETS kcm_desktoppaths DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES desktoppath.desktop DESTINATION ${SERVICES_INSTALL_DIR})

