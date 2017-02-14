project(plasma-desktop)

set(PROJECT_VERSION "5.9.2")

set(PROJECT_VERSION_MAJOR 5)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.5.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(CheckIncludeFiles)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    QuickWidgets
    DBus
    Widgets
    X11Extras
    Svg
    Concurrent
)

set(KF5_MIN_VERSION "5.24.0")
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Auth
    Plasma
    PlasmaQuick
    DocTools
    I18n
    KCMUtils
    NewStuff
    KDELibs4Support
    Notifications
    NotifyConfig
    Attica
    Wallet
    Runner
    GlobalAccel
    Declarative
    People
    DBusAddons
    Activities
    ActivitiesStats
    Config
)

find_package(LibKWorkspace CONFIG REQUIRED)
find_package(LibTaskManager CONFIG REQUIRED)
find_package(KWinDBusInterface CONFIG REQUIRED)
find_package(ScreenSaverDBusInterface CONFIG REQUIRED)
find_package(KRunnerAppDBusInterface CONFIG REQUIRED)
find_package(KSMServerDBusInterface CONFIG REQUIRED)
find_package(KF5ItemModels CONFIG REQUIRED)
find_package(KF5Emoticons CONFIG REQUIRED)

find_package(AppStreamQt 0.10.4)
set_package_properties(AppStreamQt PROPERTIES DESCRIPTION "Appstream integration"
                       TYPE RECOMMENDED
                       PURPOSE "Needed to allow appstream integration from application menus"
                      )
find_package(KF5Baloo)
set_package_properties(KF5Baloo PROPERTIES DESCRIPTION "File Searching"
                       TYPE RECOMMENDED
                       PURPOSE "Needed to build the File Search KCM"
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

find_package(UDev)
set_package_properties(UDev PROPERTIES DESCRIPTION "UDev library"
                        URL "http://www.kernel.org/pub/linux/utils/kernel/hotplug/udev.html"
                        TYPE OPTIONAL
                        PURPOSE "Required for device discovery in keyboard daemon"
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

find_package(Evdev)
set_package_properties(Evdev PROPERTIES TYPE OPTIONAL)
add_feature_info("Evdev" EVDEV_FOUND "Evdev driver headers needed for input KCM")

find_package(Synaptics)
set_package_properties(Synaptics PROPERTIES TYPE OPTIONAL)
add_feature_info("Synaptics" SYNAPTICS_FOUND "Synaptics libraries needed for touchpad KCM")
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

find_package(Breeze ${PROJECT_VERSION} CONFIG)
set_package_properties(Breeze PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "For setting the default window decoration plugin")
if(${Breeze_FOUND})
    if(${BREEZE_WITH_KDECORATION})
        set(HAVE_BREEZE_DECO true)
    else()
        set(HAVE_BREEZE_DECO FALSE)
    endif()
else()
    set(HAVE_BREEZE_DECO FALSE)
endif()

if(${AppStreamQt_FOUND})
    set(HAVE_APPSTREAMQT true)
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-appstream.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-appstream.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)
configure_file(config-runtime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-runtime.h)

plasma_install_package(desktoppackage org.kde.plasma.desktop shells shell)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

add_subdirectory(layout-templates)

add_subdirectory(doc)
add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(kcms)
add_subdirectory(knetattach)
add_subdirectory(attica-kde)
add_subdirectory(imports/activitymanager/)
add_subdirectory(solid-device-automounter)
if(X11_Xkb_FOUND AND XCB_XKB_FOUND)
    add_subdirectory(kaccess)
endif()
install(FILES org.kde.plasmashell.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
