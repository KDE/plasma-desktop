set(kio_fonts_PART_SRCS FontInstInterface.cpp KioFonts.cpp ${libkfontinstdbusiface_SRCS})
ecm_qt_declare_logging_category(kio_fonts_PART_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KFONTINST_KIO
    CATEGORY_NAME org.kde.kcm_kfontinst.kio)
# qt5_add_dbus_interface(kio_fonts_PART_SRCS ../dbus/org.kde.fontinst.xml FontinstIface)
add_library(kio_fonts MODULE ${kio_fonts_PART_SRCS} ${KFI_FONTINST_AUTH_SRC} )
target_link_libraries(kio_fonts Qt5::DBus Qt5::X11Extras Qt5::Xml KF5::Archive KF5::KIOCore KF5::KIOWidgets kfontinst )
install(TARGETS kio_fonts  DESTINATION ${KDE_INSTALL_PLUGINDIR} )
install( FILES fonts.desktop  DESTINATION  ${KDE_INSTALL_DATADIR}/konqsidebartng/virtual_folders/services )
install( FILES fonts.protocol  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
ecm_install_icons( ICONS
                   128-mimetypes-fonts-package.png
                   16-mimetypes-fonts-package.png
                   22-mimetypes-fonts-package.png
                   32-mimetypes-fonts-package.png
                   48-mimetypes-fonts-package.png
                   64-mimetypes-fonts-package.png
                   DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor )
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
