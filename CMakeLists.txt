project(plasma-desktop)
set(PROJECT_VERSION "5.2.2")
set(PROJECT_VERSION_MAJOR 5)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(FeatureSummary)
include(CheckIncludeFiles)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    QuickWidgets
    DBus
    Widgets
    X11Extras
    Svg
    Test
    Concurrent
)

set(KF5_MIN_VERSION "5.2.0")
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Auth
    Plasma
    PlasmaQuick
    DocTools
    I18n
    KCMUtils
    NewStuff
    KDELibs4Support
    NotifyConfig
    Attica
    Wallet
    Runner
    GlobalAccel
)

find_package(LibKWorkspace CONFIG REQUIRED)
find_package(LibTaskManager CONFIG REQUIRED)
find_package(KWinDBusInterface CONFIG REQUIRED)
find_package(ScreenSaverDBusInterface CONFIG REQUIRED)
find_package(KRunnerAppDBusInterface CONFIG REQUIRED)
find_package(KSMServerDBusInterface CONFIG REQUIRED)
find_package(KF5ItemModels CONFIG REQUIRED)
find_package(KF5Emoticons CONFIG REQUIRED)
find_package(PackageKitQt5 0.9)
set_package_properties(PackageKitQt5 PROPERTIES DESCRIPTION "Software Manager integration"
                       TYPE OPTIONAL
                       PURPOSE "Provides package management integration to the application launcher."
                      )

find_package(KF5Activities ${KF5_VERSION})
set_package_properties(KF5Activities PROPERTIES DESCRIPTION "management of Plasma activities"
                       TYPE OPTIONAL
                       PURPOSE "Needed by activity related plasmoids and the pager."
                      )

# Baloo is currently part of the KF5 namespace, but it is not
# a proper framework. Hence the strange versioning.
# This will be fixed with Plasma 5.2, as Baloo should have
# become a framework by then
find_package(KF5Baloo "5.1.90")
set_package_properties(KF5Baloo PROPERTIES DESCRIPTION "File Searching"
                       TYPE REQUIRED
                       PURPOSE "Needed to build to File Search KCM"
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
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace"
                      )

find_package(XCB
    REQUIRED COMPONENTS
        XCB SHM IMAGE
    OPTIONAL_COMPONENTS
        XKB XINPUT
)
set_package_properties(XCB PROPERTIES TYPE REQUIRED)
add_feature_info("XCB-XKB" XCB_XKB_FOUND "Required for building kcm/keyboard")
add_feature_info("libxft" X11_Xft_FOUND "X FreeType interface library required for font installation")

include(ConfigureChecks.cmake)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    find_package(OpenGL)
    set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                        URL "http://www.opengl.org"
                        TYPE REQUIRED
                        )
else()
    find_package(OpenGLES)
    set_package_properties(OpenGLES PROPERTIES DESCRIPTION "The OpenGLES libraries"
                        URL "http://www.khronos.org/opengles"
                        TYPE REQUIRED
                        )
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)
configure_file(config-runtime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-runtime.h)

plasma_install_package(desktoppackage org.kde.plasma.desktop shells shell)

#install(DIRECTORY layout-templates DESTINATION ${DATA_INSTALL_DIR})
plasma_install_package(layout-templates/org.kde.plasma.desktop.defaultPanel org.kde.plasma.desktop.defaultPanel layout-templates layout-template)

add_subdirectory(doc)
add_subdirectory(ktouchpadenabler)
add_subdirectory(solid-actions-kcm)

# add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(kcms)
add_subdirectory(knetattach)
add_subdirectory(attica-kde)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
