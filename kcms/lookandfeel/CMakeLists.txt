# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_lookandfeel\")

if(X11_Xcursor_FOUND)
    set(HAVE_XCURSOR TRUE)
endif()
configure_file (config-kcm.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcm.h )

set(kcm_lookandfeel_SRCS
    kcmmain.cpp
    kcm.cpp
    lookandfeeldata.cpp
    ../krdb/krdb.cpp
)

if (X11_Xcursor_FOUND)
    set(kcm_lookandfeel_SRCS
        ${kcm_lookandfeel_SRCS}
        ../cursortheme/xcursor/cursortheme.cpp
        ../cursortheme/xcursor/xcursortheme.cpp
    )
endif ()

kconfig_add_kcfg_files(kcm_lookandfeel_SRCS lookandfeelsettings.kcfgc GENERATE_MOC)

add_library(kcm_lookandfeel MODULE ${kcm_lookandfeel_SRCS})

target_link_libraries(kcm_lookandfeel
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::GuiAddons
  KF5::KCMUtils
  KF5::I18n
  KF5::Plasma
  KF5::PlasmaQuick
  KF5::KDELibs4Support
  KF5::Declarative
  KF5::QuickAddons
  PW::KWorkspace

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

kcoreaddons_desktop_to_json(kcm_lookandfeel "kcm_lookandfeel.desktop" SERVICE_TYPES kcmodule.desktop)

#this desktop file is installed only for retrocompatibility with sycoca
install(FILES lookandfeelsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kcm_lookandfeel.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_lookandfeel DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install(FILES lookandfeel.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

if(BUILD_TESTING)
   find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
   add_subdirectory(autotests)
endif()

kpackage_install_package(package kcm_lookandfeel kcms)

###### Command line tool

set(lookandfeeltool_SRCS
    lnftool.cpp
    # TODO: load kcm plugin instead of using code copy
    kcm.cpp
    lookandfeeldata.cpp
    ../krdb/krdb.cpp
)

if (X11_Xcursor_FOUND)
    set(lookandfeeltool_SRCS
        ${lookandfeeltool_SRCS}
        ../cursortheme/xcursor/cursortheme.cpp
        ../cursortheme/xcursor/xcursortheme.cpp
    )
endif ()

kconfig_add_kcfg_files(lookandfeeltool_SRCS lookandfeelsettings.kcfgc GENERATE_MOC)

add_executable(lookandfeeltool ${lookandfeeltool_SRCS})

target_link_libraries(lookandfeeltool
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::I18n
  #TODO:kpackage
  KF5::Plasma
  KF5::PlasmaQuick
  KF5::KDELibs4Support
  KF5::Declarative
  KF5::QuickAddons
  KF5::NewStuff
  PW::KWorkspace

  Qt5::DBus
  Qt5::Widgets
  Qt5::QuickWidgets
  ${X11_LIBRARIES}
)

if(X11_FOUND)
    target_link_libraries(lookandfeeltool Qt5::X11Extras)
endif()
if (X11_Xcursor_FOUND)
   target_link_libraries(lookandfeeltool ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(lookandfeeltool ${X11_Xfixes_LIB})
endif ()

install( TARGETS lookandfeeltool ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

