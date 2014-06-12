# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcminput\")

add_subdirectory( pics )

configure_file (config-kcontrol-input.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcontrol-input.h )

include_directories( ${LIBUSB_INCLUDE_DIR} )

if(X11_Xcursor_FOUND)
  set( libnoinst_SRCS
    xcursor/themepage.cpp
    xcursor/thememodel.cpp
    xcursor/cursortheme.cpp
    xcursor/xcursortheme.cpp
    xcursor/previewwidget.cpp
    xcursor/itemdelegate.cpp
    xcursor/sortproxymodel.cpp
    ../krdb/krdb.cpp )
  ki18n_wrap_ui( libnoinst_SRCS xcursor/themepage.ui )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/xcursor/ )
else()
  set( libnoinst_SRCS core/themepage.cpp )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/core/ )
endif()

set(klauncher_xml ${KINIT_DBUS_INTERFACES_DIR}/kf5_org.kde.KLauncher.xml)
qt5_add_dbus_interface(libnoinst_SRCS ${klauncher_xml} klauncher_iface)


########### next target ###############

set(kapplymousetheme_SRCS kapplymousetheme.cpp )


add_executable(kapplymousetheme ${kapplymousetheme_SRCS})

target_link_libraries(kapplymousetheme ${X11_Xrender_LIB} ${X11_X11_LIB})
if (X11_Xcursor_FOUND)
   target_link_libraries(kapplymousetheme ${X11_Xcursor_LIB})
endif ()

install(TARGETS kapplymousetheme ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcm_input_PART_SRCS mouse.cpp main.cpp logitechmouse.cpp)


ki18n_wrap_ui(kcm_input_PART_SRCS kmousedlg.ui logitechmouse_base.ui )
qt5_add_dbus_interface(kcm_input_PART_SRCS ${klauncher_xml} klauncher_iface)

add_library(kcm_input MODULE ${kcm_input_PART_SRCS})

target_link_libraries(kcm_input
    Qt5::DBus
    Qt5::X11Extras
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDELibs4Support
    ${X11_LIBRARIES}
    ${LIBUSB_LIBRARIES}
    )
if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_input ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(kcm_input ${X11_Xfixes_LIB})
endif ()

install(TARGETS kcm_input  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kcm_cursortheme_PART_SRCS kcmcursortheme.cpp ${libnoinst_SRCS})

add_library(kcm_cursortheme MODULE ${kcm_cursortheme_PART_SRCS})


target_link_libraries(kcm_cursortheme
    Qt5::DBus
    Qt5::X11Extras
    KF5::Archive
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDELibs4Support
    KF5::NewStuff
    ${X11_LIBRARIES}
    XCB::XCB
)

if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_cursortheme ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(kcm_cursortheme ${X11_Xfixes_LIB})
endif ()

install(TARGETS kcm_cursortheme  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES mouse.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES cursortheme.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz  DESTINATION  ${DATA_INSTALL_DIR}/kcminput )
install( FILES xcursor/xcursor.knsrc DESTINATION  ${CONFIG_INSTALL_DIR} )

