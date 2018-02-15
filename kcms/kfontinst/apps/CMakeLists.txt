set(kfontinst_bin_SRCS ${libkfontinstjobrunner_SRCS} ${libkfontinstdbusiface_SRCS} Installer.cpp )
# qt5_add_dbus_interface(kfontinst_bin_SRCS ../dbus/org.kde.fontinst.xml FontInstInterfaceBase)

set(kfontprint_bin_SRCS ${libkfontinstactionlabel_SRCS} Printer.cpp )
set(kfontview_bin_SRCS Viewer.cpp )

add_executable(kfontinst_bin ${kfontinst_bin_SRCS})
add_executable(kfontprint_bin ${kfontprint_bin_SRCS})
add_executable(kfontview_bin ${kfontview_bin_SRCS})

set_target_properties(kfontinst_bin PROPERTIES OUTPUT_NAME kfontinst)
set_target_properties(kfontprint_bin PROPERTIES OUTPUT_NAME kfontprint)
set_target_properties(kfontview_bin PROPERTIES OUTPUT_NAME kfontview)

target_link_libraries(kfontinst_bin
    Qt5::X11Extras
    KF5::Archive
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::XmlGui
    ${X11_X11_LIB}
    kfontinst
)
target_link_libraries(kfontprint_bin
    Qt5::PrintSupport
    KF5::IconThemes
    ${X11_X11_LIB}
    ${FREETYPE_LIBRARIES}
    kfontinstui
    kfontinst
)
target_link_libraries(kfontview_bin KF5::Parts KF5::XmlGui KF5::DBusAddons kfontinstui kfontinst )

install(TARGETS kfontinst_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kfontprint_bin DESTINATION ${KDE_INSTALL_LIBEXECDIR} )
install(TARGETS kfontview_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES kfontviewui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kfontview )
install(PROGRAMS org.kde.kfontview.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES installfont.desktop DESTINATION
${KDE_INSTALL_KSERVICES5DIR}/ServiceMenus )

ecm_install_icons( ICONS
                   16-apps-kfontview.png
                   22-apps-kfontview.png
                   32-apps-kfontview.png
                   48-apps-kfontview.png
                   64-apps-kfontview.png
                   DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)
