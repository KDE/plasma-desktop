add_library(kcm_kded MODULE kcmkded.cpp)
target_link_libraries(kcm_kded KF5::ConfigWidgets KF5::KDELibs4Support)

install(TARGETS kcm_kded  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kcmkded.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
