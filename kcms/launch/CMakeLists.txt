


########### next target ###############

set(kcm_launch_PART_SRCS kcmlaunch.cpp )

if(${CMAKE_SOURCE_DIR} STREQUAL ${plasma-desktop_SOURCE_DIR})
    set(kwin_xml ${KWIN_EFFECTS_INTERFACE})
    set(kdesktop_xml  ${KRUNNERAPP_INTERFACE})
else()
    set(kdesktop_xml  ${CMAKE_SOURCE_DIR}/plasma-workspace/krunner/dbus/org.kde.krunner.App.xml)
    set(kwin_xml ${CMAKE_SOURCE_DIR}/kwin/org.kde.kwin.Effects.xml)
endif()

qt5_add_dbus_interface( kcm_launch_PART_SRCS ${kdesktop_xml} krunner_interface )
set_source_files_properties(${kwin_xml} PROPERTIES INCLUDE "interface_util.h")
qt5_add_dbus_interface( kcm_launch_PART_SRCS ${kwin_xml} kwin_interface )



add_library(kcm_launch MODULE ${kcm_launch_PART_SRCS})


target_link_libraries(kcm_launch
    Qt5::DBus
    KF5::KCMUtils
    KF5::I18n
    KF5::KDE4Support
)

install(TARGETS kcm_launch  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmlaunch.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

