# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_cursortheme\")

include_directories( ${LIBUSB_INCLUDE_DIR} )


set( libnoinst_SRCS
xcursor/thememodel.cpp
xcursor/cursortheme.cpp
xcursor/xcursortheme.cpp
xcursor/previewwidget.cpp
xcursor/sortproxymodel.cpp
../krdb/krdb.cpp )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/xcursor/ )

########### next target ###############

set(kcm_cursortheme_PART_SRCS kcmcursortheme.cpp cursorthemedata.cpp ${libnoinst_SRCS})

kconfig_add_kcfg_files(kcm_cursortheme_PART_SRCS cursorthemesettings.kcfgc GENERATE_MOC)
add_library(kcm_cursortheme MODULE ${kcm_cursortheme_PART_SRCS})


target_link_libraries(kcm_cursortheme
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Quick
    KF5::Archive
    KF5::KCMUtils
    KF5::I18n
    KF5::GuiAddons
    KF5::WindowSystem
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDELibs4Support
    KF5::NewStuffCore
    KF5::QuickAddons
    ${X11_LIBRARIES}
    XCB::XCB
    PW::KWorkspace
)

if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_cursortheme ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(kcm_cursortheme ${X11_Xfixes_LIB})
endif ()

install(TARGETS kcm_cursortheme  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms )

kcoreaddons_desktop_to_json(kcm_cursortheme "kcm_cursortheme.desktop")

########### install files ###############

install(FILES cursorthemesettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES delete_cursor_old_default_size.upd delete_cursor_old_default_size.pl DESTINATION ${KDE_INSTALL_DATADIR}/kconf_update)
install( FILES kcm_cursortheme.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
install( FILES xcursor/xcursor.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR} )

kpackage_install_package(package kcm_cursortheme kcms)

