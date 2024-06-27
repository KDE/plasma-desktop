add_definitions(-DTRANSLATION_DOMAIN=\"kcm_keyboard\")

add_subdirectory(tastenbrett)

option(HAVE_XINPUT "X11 XInput" OFF)
option(HAVE_UDEV "UDev" OFF)

if (X11_Xinput_FOUND)
    set(HAVE_XINPUT ON)
elseif (UDEV_FOUND)
    set(HAVE_UDEV ON)
else()
    message(STATUS "X11 XInput and UDev is not found, either is required to keep layouts with keyboard hotplugging")
endif()

### kded daemon ###

set( kded_keyboard_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/debug.cpp
    keyboard_daemon.cpp
    layout_memory.cpp
    layout_memory_persister.cpp
    udev_helper.cpp
    x11_helper.cpp
    xinput_helper.cpp
    xkb_helper.cpp
    keyboard_config.cpp
    keyboard_hardware.cpp
    bindings.cpp
    flags.cpp
    xkb_rules.cpp
    layoutnames.cpp
)

kconfig_add_kcfg_files(kded_keyboard_SRCS keyboardsettings.kcfgc GENERATE_MOC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

kcoreaddons_add_plugin(kded_keyboard SOURCES ${kded_keyboard_SRCS} INSTALL_NAMESPACE kf6/kded)
set_target_properties(kded_keyboard PROPERTIES OUTPUT_NAME keyboard)

target_link_libraries(kded_keyboard
    Qt::Concurrent
    Qt::Widgets
    Qt::Xml
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::DBusAddons
    KF6::GlobalAccel
    KF6::I18n
    KF6::Notifications
    KF6::WindowSystem
    Plasma::Plasma
    KF6::XmlGui
    KF6::GuiAddons
    XCB::XCB
    XCB::XKB
    X11::X11
    X11::xkbfile
    PkgConfig::XKBREGISTRY
 )
target_link_libraries(kded_keyboard Qt::GuiPrivate)

if (HAVE_XINPUT)
    target_link_libraries(kded_keyboard X11::Xi)
endif()

if (HAVE_UDEV)
    target_link_libraries(kded_keyboard UDev::UDev)
endif()

### kcm keyboard ###
kcmutils_add_qml_kcm(kcm_keyboard SOURCES kcmmain.cpp)

set( kcm_keyboard_PART_SRCS
    kcm_keyboard.cpp
    keyboard_config.cpp
    keyboard_dbus.h
    bindings.cpp
    flags.cpp
    tastenbrett.cpp
    x11_helper.cpp
    xkb_helper.cpp
    xkb_rules.cpp
    keyboardmodel.cpp
    shortcuthelper.cpp
    xkboptionsmodel.cpp
    layoutmodel.cpp
    userlayoutmodel.cpp
)

ecm_qt_declare_logging_category(kcm_keyboard_PART_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KEYBOARD
    CATEGORY_NAME org.kde.kcm_keyboard
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KCM and KDED for keyboard layouts"
    EXPORT kcm_keyboard
)

ecm_qt_install_logging_categories(
    EXPORT kcm_keyboard
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

kconfig_add_kcfg_files(kcm_keyboard_PART_SRCS workspace_options.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_keyboard_PART_SRCS keyboardsettings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_keyboard_PART_SRCS keyboardmiscsettings.kcfgc GENERATE_MOC)

kcmutils_generate_module_data(kcm_keyboard
    MODULE_DATA_HEADER keyboardsettingsdata.h
    MODULE_DATA_CLASS_NAME KeyboardSettingsData
    SETTINGS_HEADERS workspace_options.h keyboardmiscsettings.h keyboardsettings.h
    SETTINGS_CLASSES WorkspaceOptions KeyboardMiscSettings KeyboardSettings
)

add_library(kcm_keyboard_static STATIC ${kcm_keyboard_PART_SRCS})
set_property(TARGET kcm_keyboard_static PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(kcm_keyboard_static
    Qt::Concurrent
    Qt::Widgets
    Qt::Xml
    Qt::GuiPrivate
    KF6::KCMUtils
    KF6::GlobalAccel
    KF6::I18n
    KF6::WindowSystem
    Plasma::Plasma
    KF6::XmlGui
    KF6::GuiAddons
    XCB::XCB
    XCB::XKB
    X11::X11
    X11::xkbfile
    PkgConfig::XKBREGISTRY
)

target_link_libraries(kcm_keyboard kcm_keyboard_static)

configure_file(config-keyboard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-keyboard.h)

# Unit tests
if(BUILD_TESTING)
    add_subdirectory( tests )
endif()

add_dependencies(kcm_keyboard tastenbrett)

add_subdirectory(qml)
