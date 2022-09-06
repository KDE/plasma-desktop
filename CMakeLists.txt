cmake_minimum_required(VERSION 3.16)

project(plasma-desktop)

set(PROJECT_VERSION "5.25.5")
set(PROJECT_VERSION_MAJOR 5)

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.94")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(ACCOUNTSQT_DEP_VERSION "1.13")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(CheckIncludeFiles)
include(KDEClangFormat)
include(KDEGitCommitHooks)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    QuickWidgets
    DBus
    Widgets
    Svg
    Concurrent
)
if (QT_MAJOR_VERSION EQUAL "5")
    find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS X11Extras)
endif()

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Auth
    Crash
    Plasma
    PlasmaQuick
    DocTools
    I18n
    KCMUtils
    NewStuff
    NewStuffQuick
    KIO
    Notifications
    NotifyConfig
    Attica
    Runner
    GlobalAccel
    CoreAddons # KSharedDataCache required by KImageCache, KStringHandler required by FolderModel
    GuiAddons # KImageCache
    Declarative
    DBusAddons
    Activities
    ActivitiesStats
    Config
    WidgetsAddons
    Codecs
    Sonnet
    KDELibs4Support # kcms/dateandtime
)

find_package(AccountsQt${QT_MAJOR_VERSION} ${ACCOUNTSQT_DEP_VERSION} CONFIG)

find_package(KUserFeedback)

set_package_properties(AccountsQt${QT_MAJOR_VERSION} PROPERTIES
    DESCRIPTION "Accounts management library for Qt applications"
    URL "https://gitlab.com/accounts-sso/libaccounts-qt"
    PURPOSE "Required for building the OpenDesktop integration plugin"
    TYPE OPTIONAL
)

find_package(KAccounts 20.04)
set_package_properties(KAccounts PROPERTIES
    DESCRIPTION "Accounts management library for KDE applications"
    PURPOSE "Required for building the OpenDesktop integration plugin"
    TYPE OPTIONAL
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SignOnOAuth2 IMPORTED_TARGET signon-oauth2plugin)
set_package_properties(signon-oauth2plugin PROPERTIES
    DESCRIPTION "Plugin for SignOnQt${QT_MAJOR_VERSION} which handles OAuth and OAuth2 logins"
    URL "https://gitlab.com/accounts-sso/signon-plugin-oauth2"
    PURPOSE "Required for running the OpenDesktop integration plugin"
    TYPE RUNTIME
)

find_package(KF5Kirigami2 ${KF5_MIN_VERSION} CONFIG)
set_package_properties(KF5Kirigami2 PROPERTIES
    DESCRIPTION "A QtQuick based components set"
    PURPOSE "Required at runtime by many KCMs"
    TYPE RUNTIME
)

find_package(KF5QQC2DesktopStyle ${KF5_MIN_VERSION} CONFIG)
set_package_properties(KF5QQC2DesktopStyle PROPERTIES
    DESCRIPTION "QtQuickControls 2 style that uses QWidget's QStyle for painting"
    PURPOSE "Required at runtime by many KCMs"
    TYPE RUNTIME
)

find_package(LibKWorkspace ${PROJECT_VERSION} CONFIG REQUIRED)
find_package(LibNotificationManager ${PROJECT_VERSION} CONFIG REQUIRED)
find_package(LibTaskManager ${PROJECT_VERSION} CONFIG REQUIRED)
find_package(LibColorCorrect ${PROJECT_VERSION} CONFIG REQUIRED)
find_package(KWinDBusInterface CONFIG REQUIRED)
find_package(ScreenSaverDBusInterface CONFIG REQUIRED)
find_package(KRunnerAppDBusInterface CONFIG REQUIRED)
find_package(KSMServerDBusInterface CONFIG REQUIRED)
find_package(KF5ItemModels CONFIG REQUIRED)
find_package(KSysGuard CONFIG REQUIRED)
find_package(KDED CONFIG REQUIRED)

find_package(KF5Baloo ${KF5_MIN_VERSION})
set_package_properties(KF5Baloo PROPERTIES
    DESCRIPTION "File Searching"
    PURPOSE "Needed to build the File Search KCM"
    TYPE RECOMMENDED
)

find_package(X11)
set_package_properties(X11 PROPERTIES
    DESCRIPTION "X11 libraries"
    URL "https://www.x.org"
    PURPOSE "Required for building the X11 based workspace"
    TYPE REQUIRED
)

if(X11_FOUND)
  set(HAVE_X11 1)
endif()

find_package(UDev)
set_package_properties(UDev PROPERTIES
    DESCRIPTION "API for enumerating and introspecting local devices (part of systemd)"
    URL "https://www.freedesktop.org/wiki/Software/systemd/"
    PURPOSE "Required for device discovery in keyboard daemon"
    TYPE OPTIONAL
)

find_package(XCB
    REQUIRED COMPONENTS
        XCB SHM IMAGE
    OPTIONAL_COMPONENTS
        XKB XINPUT ATOM RECORD
)

find_package(X11_XCB)
set_package_properties(X11_XCB PROPERTIES
    PURPOSE "Required for needed for touchpad KCM (X11 backend)"
    TYPE OPTIONAL
)

set_package_properties(XCB PROPERTIES TYPE REQUIRED)
add_feature_info("XCB-XKB" XCB_XKB_FOUND "Required for building kcm/keyboard")
add_feature_info("libxft" X11_Xft_FOUND "X FreeType interface library required for font installation")

pkg_check_modules(XORGLIBINPUT xorg-libinput IMPORTED_TARGET)
pkg_check_modules(EVDEV xorg-evdev>=2.8.99.1 IMPORTED_TARGET)
pkg_check_modules(XORGSERVER xorg-server IMPORTED_TARGET)

find_package(Synaptics)
set_package_properties(Synaptics PROPERTIES TYPE OPTIONAL)
add_feature_info("Synaptics" SYNAPTICS_FOUND "Synaptics libraries are needed for the synaptics touchpad KCM")

if(SYNAPTICS_FOUND)
    set(HAVE_SYNAPTICS 1)
endif()

if(XORGLIBINPUT_FOUND)
  set(HAVE_XORGLIBINPUT 1)
endif()

include(ConfigureChecks.cmake)

find_package(PackageKitQt${QT_MAJOR_VERSION})
set_package_properties(PackageKitQt${QT_MAJOR_VERSION} PROPERTIES
    DESCRIPTION "Software Manager integration"
    PURPOSE "Used in the KRunner plugin installer"
    TYPE OPTIONAL
)

if(PackageKitQt${QT_MAJOR_VERSION}_FOUND)
    set(HAVE_PACKAGEKIT TRUE)
endif()

find_package(GLIB2)
set_package_properties(GLIB2 PROPERTIES
    PURPOSE "Required for building kimpanel"
    TYPE OPTIONAL
)

# For kimpanel
pkg_check_modules(IBUS "ibus-1.0>=1.5.0" IMPORTED_TARGET)
pkg_check_modules(GOBJECT gobject-2.0 IMPORTED_TARGET)
pkg_check_modules(SCIM scim IMPORTED_TARGET)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX PLASMA VERSION_HEADER plasma_version.h)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)
configure_file(config-runtime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-runtime.h)

plasma_install_package(desktoppackage org.kde.plasma.desktop shells shell)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING -DQT_NO_FOREACH)

add_subdirectory(layout-templates)

add_subdirectory(doc)
add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(kcms)
add_subdirectory(knetattach)
if(KAccounts_FOUND AND AccountsQt${QT_MAJOR_VERSION}_FOUND)
    add_subdirectory(attica-kde)
endif()
add_subdirectory(imports/activitymanager/)
add_subdirectory(solid-device-automounter)
if(X11_Xkb_FOUND AND XCB_XKB_FOUND)
    add_subdirectory(kaccess)
endif()
install(FILES org.kde.plasmashell.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

find_package(XdgUserDir)
set_package_properties(XdgUserDir PROPERTIES
    DESCRIPTION "Xdg-user-dir script"
    PURPOSE "Used to translate default paths in KCM desktoppath"
    TYPE RUNTIME
)
