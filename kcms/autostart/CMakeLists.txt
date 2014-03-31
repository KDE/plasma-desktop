project (autostart)

set(kcm_autostart_PART_SRCS
   autostartitem.cpp
   addscriptdialog.cpp
   advanceddialog.cpp
   autostart.cpp )

qt5_wrap_ui(kcm_autostart_PART_SRCS autostartconfig.ui )

add_library(kcm_autostart MODULE ${kcm_autostart_PART_SRCS})

target_link_libraries(kcm_autostart KF5::KCMUtils KF5::I18n KF5::KIOCore KF5::KIOWidgets)

########### install files ###############

install(TARGETS kcm_autostart  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES autostart.desktop   DESTINATION  ${SERVICES_INSTALL_DIR} )
