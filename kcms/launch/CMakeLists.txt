# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmlaunch\")


########### next target ###############

set(kcm_launch_PART_SRCS kcmlaunch.cpp )

set_source_files_properties(${KWIN_EFFECTS_INTERFACE} PROPERTIES INCLUDE "interface_util.h")
qt5_add_dbus_interface( kcm_launch_PART_SRCS ${KWIN_EFFECTS_INTERFACE} kwin_interface )



add_library(kcm_launch MODULE ${kcm_launch_PART_SRCS})


target_link_libraries(kcm_launch
    Qt5::DBus
    KF5::KCMUtils
    KF5::I18n
)

install(TARGETS kcm_launch  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmlaunch.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

