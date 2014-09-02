# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_lookandfeel\")

configure_file (config-kcm.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcm.h )


set(kcm_lookandfeel_SRCS
  kcm.cpp
  ../krdb/krdb.cpp
  ../input/xcursor/cursortheme.cpp
  ../input/xcursor/xcursortheme.cpp
)

set(klauncher_xml ${KINIT_DBUS_INTERFACES_DIR}/kf5_org.kde.KLauncher.xml)
qt5_add_dbus_interface(kcm_lookandfeel_SRCS ${klauncher_xml} klauncher_iface)

add_library(kcm_lookandfeel MODULE ${kcm_lookandfeel_SRCS})

target_link_libraries(kcm_lookandfeel
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::I18n
  KF5::Plasma
  KF5::PlasmaQuick
  KF5::KDELibs4Support

  Qt5::DBus
  Qt5::Widgets
  Qt5::QuickWidgets
  ${X11_LIBRARIES}
)

if(X11_FOUND)
    target_link_libraries(kcm_lookandfeel Qt5::X11Extras)
endif()
if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_lookandfeel ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(kcm_lookandfeel ${X11_Xfixes_LIB})
endif ()

install(FILES kcm_lookandfeel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kcm_lookandfeel DESTINATION ${PLUGIN_INSTALL_DIR})

add_subdirectory(autotests)

plasma_install_package(package kcm_lookandfeel kcms kcm_lookandfeel)
