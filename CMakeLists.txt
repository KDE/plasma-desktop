cmake_minimum_required(VERSION 3.16)

project(plasma-desktop)

set(PROJECT_VERSION "5.22.5")
set(PROJECT_VERSION_MAJOR 5)

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.82")
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

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    QuickWidgets
    DBus
    Widgets
    X11Extras
    Svg
    Concurrent
)

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
    CoreAddons # KSharedDataCache required by KImageCache
    GuiAddons # KImageCache
    Declarative
    DBusAddons
    Activities
    ActivitiesStats
    Config
    Wallet
    WidgetsAddons
    Codecs
    Sonnet
    KDELibs4Support # kcms/dateandtime
)

find_package(AccountsQt5 ${ACCOUNTSQT_DEP_VERSION} CONFIG)

find_package(KUserFeedback)

set_package_properties(AccountsQt5 PROPERTIES DESCRIPTION "Accounts management library for Qt applications"
   URL "https://gitlab.com/accounts-sso/libaccounts-qt"
   TYPE OPTIONAL
   PURPOSE "Required for building the OpenDesktop integration plugin")

find_package(KAccounts 20.04)
set_package_properties(KAccounts PROPERTIES DESCRIPTION "Accounts management library for KDE applications"
   TYPE OPTIONAL
   PURPOSE "Required for building the OpenDesktop integration plugin")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SignOnOAuth2 IMPORTED_TARGET signon-oauth2plugin)
set_package_properties(signon-oauth2plugin PROPERTIES DESCRIPTION "Plugin for SignOnQt5 which handles OAuth and OAuth2 logins"
   URL "https://gitlab.com/accounts-sso/signon-plugin-oauth2"
   TYPE RUNTIME
   PURPOSE "Required for running the OpenDesktop integration plugin")

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
set_package_properties(KF5Baloo PROPERTIES DESCRIPTION "File Searching"
                       TYPE RECOMMENDED
                       PURPOSE "Needed to build the File Search KCM"
                      )

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "https://www.x.org"
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace"
                      )
if(X11_FOUND)
  set(HAVE_X11 1)
endif()

find_package(UDev)
set_package_properties(UDev PROPERTIES DESCRIPTION "UDev library"
                        URL "https://www.freedesktop.org/wiki/Software/systemd/"
                        TYPE OPTIONAL
                        PURPOSE "Required for device discovery in keyboard daemon"
                      )

find_package(XCB
    REQUIRED COMPONENTS
        XCB SHM IMAGE
    OPTIONAL_COMPONENTS
        XKB XINPUT ATOM RECORD
)

find_package(X11_XCB)
set_package_properties(X11_XCB PROPERTIES TYPE OPTIONAL
                        PURPOSE "Required for needed for touchpad KCM (X11 backend)"
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

find_package(PackageKitQt5)
set_package_properties(PackageKitQt5
        PROPERTIES DESCRIPTION "Software Manager integration"
        TYPE OPTIONAL
        PURPOSE "Used in the KRunner plugin installer"
        )
if(PackageKitQt5_FOUND)
    set(HAVE_PACKAGEKIT TRUE)
endif()

find_package(GLIB2)
set_package_properties(GLIB2
        PROPERTIES TYPE OPTIONAL
        PURPOSE "Required for building kimpanel"
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

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

add_subdirectory(layout-templates)

add_subdirectory(doc)
add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(kcms)
add_subdirectory(knetattach)
if(KAccounts_FOUND AND AccountsQt5_FOUND)
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
set_package_properties(XdgUserDir PROPERTIES DESCRIPTION "Xdg-user-dir script"
    TYPE RUNTIME
    PURPOSE "Used to translate default paths in KCM desktoppath")


