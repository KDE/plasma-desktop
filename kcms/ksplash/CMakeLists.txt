# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"ksplashthemes\")

add_subdirectory( pics )

########### next target ###############

set(kcm_ksplashthemes_PART_SRCS installer.cpp main.cpp )
add_library(kcm_ksplashthemes MODULE ${kcm_ksplashthemes_PART_SRCS})
target_link_libraries(kcm_ksplashthemes KF5::KIOCore KF5::KIOWidgets KF5::NewStuff KF5::KDELibs4Support)
install(TARGETS kcm_ksplashthemes  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES ksplashthememgr.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES ksplash.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
