project(plasma-desktop)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(FeatureSummary)
include(CheckIncludeFiles)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    DBus
    Widgets
    X11Extras
    Concurrent
)

find_package(KF5 CONFIG REQUIRED COMPONENTS
    Plasma
    DocTools
    I18n
    KCMUtils
    NewStuff
    KDE4Support
    NotifyConfig
    Su
    Runner
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDecorations REQUIRED CONFIG)
    find_package(LibKWorkspace CONFIG REQUIRED)
    find_package(LibTaskManager CONFIG REQUIRED)
    find_package(KWinDBusInterface CONFIG REQUIRED)
    find_package(ScreenSaverDBusInterface CONFIG REQUIRED)
    find_package(KRunnerAppDBusInterface CONFIG REQUIRED)
    find_package(KSMServerDBusInterface CONFIG REQUIRED)
endif()

find_package(KF5Activities ${KF5_VERSION})
set_package_properties(KF5Activities PROPERTIES DESCRIPTION "management of Plasma activities"
                       TYPE OPTIONAL
                       PURPOSE "Needed by activity related plasmoids and the pager."
                      )

find_package(USB)
set(HAVE_LIBUSB ${LIBUSB_FOUND})
set_package_properties(USB PROPERTIES DESCRIPTION "User level access to USB devices"
                       URL "http://libusb.sourceforge.net"
                       TYPE OPTIONAL
                       PURPOSE "Provides Logitech mouse support in KControl."
                      )

find_package(Fontconfig)
set_package_properties(Fontconfig PROPERTIES DESCRIPTION "Font access configuration library"
                       URL "http://www.freedesktop.org/wiki/Software/fontconfig"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build font configuration and installation tools"
                      )

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE OPTIONAL
                        PURPOSE "Required for building the X11 based workspace"
                      )
if(X11_FOUND)
  find_package(XCB REQUIRED COMPONENTS XCB XFIXES DAMAGE COMPOSITE SHAPE SYNC RENDER RANDR KEYSYMS IMAGE SHM XTEST)
  set_package_properties(XCB PROPERTIES TYPE REQUIRED)
  find_package(XCB COMPONENTS ICCCM)
  add_feature_info("XCB-ICCCM" XCB_ICCCM_FOUND "Required for building test applications for KWin")
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include(ConfigureChecks.cmake)
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

set(PLASMADESKTOP_VERSION_MAJOR 4)
set(PLASMADESKTOP_VERSION_MINOR 95)
set(PLASMADESKTOP_VERSION_PATCH 0)
set(PLASMADESKTOP_VERSION ${PLASMADESKTOP_VERSION_MAJOR}.${PLASMADESKTOP_VERSION_MINOR}.${PLASMADESKTOP_VERSION_PATCH} )

set(GENERIC_LIB_SOVERSION ${PLASMADESKTOP_VERSION_MAJOR})
set(GENERIC_LIB_VERSION "${PLASMADESKTOP_VERSION_MAJOR}.${PLASMADESKTOP_VERSION_MINOR}.${PLASMADESKTOP_VERSION_PATCH}")

plasma_install_package(desktoppackage org.kde.desktop plasma/shells shell)

add_subdirectory(doc)
# add_subdirectory(ktouchpadenabler)
# add_subdirectory(solid-actions-kcm)

# add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(kcms)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
