cmake_minimum_required(VERSION 3.16)

project(plasma-desktop)

set(PROJECT_VERSION "6.1.90")
set(PROJECT_VERSION_MAJOR 6)

set(PROJECT_DEP_VERSION "6.1.80")
set(QT_MIN_VERSION "6.7.0")
set(KF6_MIN_VERSION "6.5.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.85")
set(ACCOUNTSQT_DEP_VERSION "1.13")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(INSTALL_SDDM_THEME TRUE)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)
include(ECMGenerateExportHeader)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(CheckIncludeFiles)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMQmlModule)
include(ECMConfiguredInstall)

option(BUILD_KCM_MOUSE_KWIN_WAYLAND "Build the Mouse KCM's KWin+Wayland backend" ON)
option(BUILD_KCM_MOUSE_X11 "Build the Mouse KCM's X11 backend" ON)
option(BUILD_KCM_TOUCHPAD_KWIN_WAYLAND "Build the Touchpad KCM's KWin+Wayland backend" ON)
option(BUILD_KCM_TOUCHPAD_X11 "Build the Touchpad KCM's X11 backend" ON)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    QuickWidgets
    DBus
    Widgets
    Svg
    Concurrent
    Core5Compat
    Sql # kcms/activities
)
find_package(Qt6 ${QT_MIN_VERSION} CONFIG OPTIONAL_COMPONENTS WaylandClient)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Auth
    Crash
    Config
    DocTools
    I18n
    KCMUtils
    NewStuff
    KIO
    Notifications
    NotifyConfig
    Attica
    Runner
    GlobalAccel
    CoreAddons # KSharedDataCache required by KImageCache, KStringHandler required by FolderModel
    GuiAddons # KImageCache
    DBusAddons
    Config
    WidgetsAddons
    Codecs
    Sonnet
    Package
    IconThemes
    XmlGui
    Svg
)

find_package(Plasma5Support ${PROJECT_DEP_VERSION} REQUIRED)
find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)
find_package(PlasmaQuick ${PROJECT_DEP_VERSION} REQUIRED)
find_package(PlasmaActivities ${PROJECT_DEP_VERSION} REQUIRED)
find_package(PlasmaActivitiesStats ${PROJECT_DEP_VERSION} REQUIRED)

if(BUILD_TESTING)
    find_package(Qt6Test ${QT_MIN_VERSION} CONFIG REQUIRED)
    include(ECMAddTests)
endif()

find_package(AccountsQt6 ${ACCOUNTSQT_DEP_VERSION} CONFIG)

set_package_properties(AccountsQt6 PROPERTIES
    DESCRIPTION "Accounts management library for Qt applications"
    URL "https://gitlab.com/accounts-sso/libaccounts-qt"
    PURPOSE "Required for building the OpenDesktop integration plugin"
    TYPE OPTIONAL
)

find_package(KAccounts6 20.04)
set_package_properties(KAccounts6 PROPERTIES
    DESCRIPTION "Accounts management library for KDE applications"
    PURPOSE "Required for building the OpenDesktop integration plugin"
    TYPE OPTIONAL
)


find_package(PkgConfig REQUIRED)
pkg_check_modules(SignOnOAuth2 IMPORTED_TARGET signon-oauth2plugin)
set_package_properties(signon-oauth2plugin PROPERTIES
    DESCRIPTION "Plugin for SignOnQt6 which handles OAuth and OAuth2 logins"
    URL "https://gitlab.com/accounts-sso/signon-plugin-oauth2"
    PURPOSE "Required for running the OpenDesktop integration plugin"
    TYPE RUNTIME
)

find_package(KF6Kirigami ${KF6_MIN_VERSION} CONFIG)
set_package_properties(KF6Kirigami PROPERTIES
    DESCRIPTION "A QtQuick based components set"
    PURPOSE "Required at runtime by many KCMs"
    TYPE RUNTIME
)


find_package(KF6KirigamiAddons 1.0.0 CONFIG)
set_package_properties(KF6KirigamiAddons PROPERTIES
    DESCRIPTION "Extra controls for Kirigami applications"
    PURPOSE "Required at runtime for Kickoff"
    TYPE RUNTIME
)

find_package(KF6QQC2DesktopStyle ${KF6_MIN_VERSION} CONFIG)
set_package_properties(KF6QQC2DesktopStyle PROPERTIES
    DESCRIPTION "QtQuickControls 2 style that uses QWidget's QStyle for painting"
    PURPOSE "Required at runtime by many KCMs"
    TYPE RUNTIME
)

find_package(WaylandProtocols 1.25)
set_package_properties(WaylandProtocols PROPERTIES
    TYPE REQUIRED
    PURPOSE "Collection of Wayland protocols that add functionality not available in the Wayland core protocol"
    URL "https://gitlab.freedesktop.org/wayland/wayland-protocols/"
)
find_package(Wayland 1.2)
set_package_properties(Wayland PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for building Tablet input KCM"
)

find_package(SDL2 2.0.16)
set_package_properties(SDL2 PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Required for building Game Controller KCM"
)

find_package(LibKWorkspace ${PROJECT_DEP_VERSION} CONFIG REQUIRED)
find_package(LibNotificationManager ${PROJECT_DEP_VERSION} CONFIG REQUIRED)
find_package(LibTaskManager ${PROJECT_DEP_VERSION} CONFIG REQUIRED)
find_package(LibColorCorrect ${PROJECT_DEP_VERSION} CONFIG REQUIRED)
find_package(KWinDBusInterface CONFIG REQUIRED)
find_package(ScreenSaverDBusInterface CONFIG REQUIRED)
find_package(KRunnerAppDBusInterface CONFIG REQUIRED)
find_package(KSMServerDBusInterface CONFIG REQUIRED)
find_package(KF6ItemModels CONFIG REQUIRED)
find_package(KSysGuard CONFIG REQUIRED)
find_package(KF6KDED CONFIG REQUIRED)

find_package(KF6Baloo ${KF6_MIN_VERSION})
set_package_properties(KF6Baloo PROPERTIES
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

find_package(Canberra)
set_package_properties(Canberra PROPERTIES
    PURPOSE "Support audible bell in kaccess"
    TYPE REQUIRED)

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

if (BUILD_KCM_TOUCHPAD_X11)
    find_package(X11_XCB)
    set_package_properties(X11_XCB PROPERTIES
        PURPOSE "Required for touchpad KCM (X11 backend) when option BUILD_KCM_TOUCHPAD_X11=ON"
        TYPE REQUIRED
    )
endif()


find_package(ICU COMPONENTS i18n uc)
set_package_properties(ICU PROPERTIES
    PURPOSE "Retrieving timezone info"
    TYPE REQUIRED
)

set_package_properties(XCB PROPERTIES TYPE REQUIRED)
add_feature_info("XCB-XKB" XCB_XKB_FOUND "Required for building kcm/keyboard")
add_feature_info("libxft" X11_Xft_FOUND "X FreeType interface library required for font installation")

pkg_check_modules(XORGLIBINPUT xorg-libinput IMPORTED_TARGET)
pkg_check_modules(XORGSERVER xorg-server IMPORTED_TARGET)
pkg_check_modules(XKBREGISTRY xkbregistry REQUIRED IMPORTED_TARGET)

ecm_find_qmlmodule(org.kde.pipewire 0.1)

if(XORGLIBINPUT_FOUND)
  set(HAVE_XORGLIBINPUT 1)
endif()

include(ConfigureChecks.cmake)

find_package(PackageKitQt6)
set_package_properties(PackageKitQt6 PROPERTIES
    DESCRIPTION "Software Manager integration"
    PURPOSE "Used in the KRunner plugin installer"
    TYPE OPTIONAL
)

if(PackageKitQt6_FOUND)
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
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)
configure_file(config-build-options.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-build-options.h)

plasma_install_package(desktoppackage org.kde.plasma.desktop shells shell)

if (INSTALL_SDDM_THEME)
    configure_file(sddm-theme/theme.conf.cmake ${CMAKE_CURRENT_BINARY_DIR}/sddm-theme/theme.conf)

# Install the login theme into the SDDM directory
    #NOTE this trailing slash is important to rename the directory
    install(DIRECTORY sddm-theme/
        DESTINATION ${KDE_INSTALL_FULL_DATADIR}/sddm/themes/breeze
        PATTERN "README.txt" EXCLUDE
        PATTERN "components" EXCLUDE
        PATTERN "dummydata" EXCLUDE
        PATTERN "theme.conf.cmake" EXCLUDE
    )

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sddm-theme/theme.conf
        DESTINATION ${KDE_INSTALL_FULL_DATADIR}/sddm/themes/breeze
    )
endif()

add_subdirectory(layout-templates)

add_subdirectory(doc)
add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(kcms)
add_subdirectory(knetattach)
if(KAccounts6_FOUND AND AccountsQt6_FOUND)
    add_subdirectory(attica-kde)
endif()
add_subdirectory(imports/activitymanager/)
add_subdirectory(solid-device-automounter)
add_subdirectory(emojier)
if(X11_Xkb_FOUND AND XCB_XKB_FOUND)
    add_subdirectory(kaccess)
endif()
install(FILES org.kde.plasmashell.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kde-mimeapps.list DESTINATION ${KDE_INSTALL_APPDIR})

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

find_package(FontNotoColorEmoji)
set_package_properties(FontNotoColorEmoji PROPERTIES
    PURPOSE "Default emoji font, needed to display emojis properly in Emoji Selector"
    URL "https://www.google.com/get/noto/"
    TYPE RUNTIME
)

add_subdirectory(appiumtests)

ki18n_install(po)
kdoctools_install(po)
