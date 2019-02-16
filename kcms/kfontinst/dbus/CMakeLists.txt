
add_definitions(${QT_DEFINITIONS})

set(fontinst_bin_SRCS FcConfig.cpp FontInst.cpp Folder.cpp Main.cpp Utils.cpp ${libkfontinstdbusiface_SRCS} )
set(fontinst_helper_SRCS FcConfig.cpp Helper.cpp Folder.cpp Utils.cpp ${libkfontinstdbusiface_SRCS} )

# qt5_generate_dbus_interface(FontInst.h org.kde.fontinst.xml)
qt5_add_dbus_adaptor(fontinst_bin_SRCS org.kde.fontinst.xml FontInst.h KFI::FontInst)
# qt5_add_dbus_interface(fontinst_bin_SRCS org.kde.fontinst.xml FontinstIface)

add_executable(fontinst_bin ${fontinst_bin_SRCS})
add_executable(fontinst_helper ${fontinst_helper_SRCS})

set_target_properties(fontinst_bin PROPERTIES OUTPUT_NAME fontinst)
target_link_libraries(fontinst_bin 
                      Qt5::DBus Qt5::Xml KF5::AuthCore ${FONTCONFIG_LIBRARIES} kfontinst)

set_target_properties(fontinst_helper PROPERTIES OUTPUT_NAME fontinst_helper)
target_link_libraries(fontinst_helper 
                      Qt5::DBus Qt5::Xml KF5::AuthCore ${FONTCONFIG_LIBRARIES} kfontinst)

install(TARGETS fontinst_bin DESTINATION ${KAUTH_HELPER_INSTALL_DIR} )
install(TARGETS fontinst_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR} )
install(PROGRAMS fontinst_x11 DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

configure_file(org.kde.fontinst.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.fontinst.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.fontinst.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR} )

kauth_install_helper_files(fontinst_helper org.kde.fontinst root)
kauth_install_actions(org.kde.fontinst fontinst.actions)
