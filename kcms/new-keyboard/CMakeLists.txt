cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOMOC ON)

set(CMAKE_INSTALL_PREFIX /usr)

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
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_keyboard\")

set(kcm_keyboard_SRCS
    main.cpp
)

add_library(kcm_keyboard MODULE ${kcm_keyboard_SRCS})

target_link_libraries(kcm_keyboard
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::QuickAddons
    KF5::Declarative
    Qt5::Quick
    KF5::KCMUtils
    KF5::I18n
)

kcoreaddons_desktop_to_json(kcm_keyboard "kcm_keyboard.desktop" SERVICE_TYPES kcmodule.desktop)
install(FILES kcm_keyboard.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_keyboard DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_keyboard kcms)
