cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOMOC ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Qml
)

find_package(KF5 REQUIRED COMPONENTS
    CoreAddons
    KCMUtils
    I18n
    Plasma
    PlasmaQuick
    Declarative
)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkeyboard\")

set(kcm_keyboard_SRCS
    kcm_keyboard.cpp
    hardware_config_model.cpp
    layout_config_model.cpp
    keyboard_model_model.cpp
    advanced_model.cpp
    advanced_filter_proxy_model.cpp
)

ecm_qt_declare_logging_category(kcm_keyboard_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KEYBOARD
    CATEGORY_NAME org.kde.kcm_keyboard
    DEFAULT_SEVERITY Warning)

add_library(kcm_keyboard MODULE ${kcm_keyboard_SRCS})

target_link_libraries(kcm_keyboard
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::QuickAddons
    KF5::Declarative
    KF5::KCMUtils
    KF5::I18n
    KF5::GlobalAccel
    KF5::WindowSystem
    KF5::IconThemes
    KF5::XmlGui
    KF5::Plasma
    KF5::ItemModels
    KF5::GuiAddons
    Qt5::Quick
    Qt5::Concurrent
    Qt5::X11Extras
    Qt5::DBus
    kcm_keyboard_layoutmodel
)

kcoreaddons_desktop_to_json(kcm_keyboard "kcm_keyboard.desktop" SERVICE_TYPES kcmodule.desktop)
install(FILES kcm_keyboard.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_keyboard DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

# need this line because the old .so will be loaded without it
install(TARGETS kcm_keyboard DESTINATION ${KDE_INSTALL_PLUGINDIR})

kpackage_install_package(package kcm_keyboard kcms)

