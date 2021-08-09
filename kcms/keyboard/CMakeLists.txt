add_definitions(-DTRANSLATION_DOMAIN=\"kcmkeyboard\")

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

include_directories(${CMAKE_CURRENT_BINARY_DIR})
kcoreaddons_add_plugin(kded_keyboard SOURCES ${kded_keyboard_SRCS} INSTALL_NAMESPACE "kf5/kded")
set_target_properties(kded_keyboard PROPERTIES OUTPUT_NAME keyboard)
kcoreaddons_desktop_to_json(kded_keyboard keyboard.desktop SERVICE_TYPES kdedmodule.desktop)

target_link_libraries(kded_keyboard
    Qt::Concurrent
    Qt::Widgets
    Qt::X11Extras
    Qt::Xml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::GlobalAccel
    KF5::I18n
    KF5::Notifications
    KF5::WindowSystem
    KF5::Plasma
    KF5::XmlGui
    KF5::GuiAddons
    XCB::XCB
    XCB::XKB
    X11::X11
    X11::xkbfile
 )

if (HAVE_XINPUT)
    target_link_libraries(kded_keyboard X11::Xi)
endif()

if (HAVE_UDEV)
    target_link_libraries(kded_keyboard ${UDEV_LIBS})
endif()

### kcm keyboard ###

set(kcm_keyboard_PART_SRCS
  kcm_keyboard.cpp
  kcm_keyboard_widget.cpp
  kcm_view_models.cpp
  kcm_add_layout_dialog.cpp
  keyboard_config.cpp
  x11_helper.cpp
  xkb_helper.cpp
  xkb_rules.cpp
  flags.cpp
  iso_codes.cpp
  kcmmisc.cpp
  bindings.cpp
  tastenbrett.cpp)

install(FILES kcminputrc_repeat.upd
        DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
install(PROGRAMS kcminputrc_migrate_repeat_value.py
        DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
install(FILES kcminputrc_fix_botched_5_21_0.upd
        DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
install(PROGRAMS kcminputrc_fix_botched_5_21_0.py
        DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})

ecm_qt_declare_logging_category(kcm_keyboard_PART_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KEYBOARD
    CATEGORY_NAME org.kde.kcm_keyboard
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KCM and KDED for keyboard layouts"
    EXPORT kcm_keyboard)
ecm_qt_install_logging_categories(
    EXPORT kcm_keyboard
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

kconfig_add_kcfg_files(kcm_keyboard_PART_SRCS workspace_options.kcfgc)

ki18n_wrap_ui(kcm_keyboard_PART_SRCS kcm_keyboard.ui kcm_add_layout_dialog.ui kcmmiscwidget.ui)

add_library(kcm_keyboard MODULE ${kcm_keyboard_PART_SRCS})

target_link_libraries(kcm_keyboard
    Qt::Concurrent
    Qt::Widgets
    Qt::X11Extras
    Qt::Xml
    KF5::KCMUtils
    KF5::GlobalAccel
    KF5::I18n
    KF5::WindowSystem
    KF5::Plasma
    KF5::XmlGui
    KF5::GuiAddons
    XCB::XCB
    XCB::XKB
    X11::X11
    X11::xkbfile
)

install(TARGETS kcm_keyboard DESTINATION ${KDE_INSTALL_PLUGINDIR} )

install( FILES kcm_keyboard.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )

add_subdirectory(kconf_update)

# Images

add_subdirectory( pics )

configure_file(config-keyboard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-keyboard.h)

# Unit tests
if(BUILD_TESTING)
    find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
    add_subdirectory( tests )
endif()

add_dependencies(kcm_keyboard tastenbrett)
