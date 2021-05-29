# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_keys\")

set(kcm_keys_SRCS
    basemodel.cpp
    kcm_keys.cpp
    filteredmodel.cpp
    globalaccelmodel.cpp
    shortcutsmodel.cpp
    standardshortcutsmodel.cpp
    keysdata.cpp
    )

set(kglobalaccel_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf5_org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES
    INCLUDE "kglobalshortcutinfo.h"
    NO_NAMESPACE TRUE
    CLASSNAME "KGlobalAccelInterface"
)
qt_add_dbus_interface(kcm_keys_SRCS ${kglobalaccel_xml} kglobalaccel_interface)

set(kglobalaccel_component_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf5_org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES
    INCLUDE "kglobalshortcutinfo.h"
    NO_NAMESPACE TRUE
    CLASSNAME "KGlobalAccelComponentInterface"
)
qt_add_dbus_interface(kcm_keys_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface)


ecm_qt_declare_logging_category(kcm_keys_SRCS
    HEADER kcmkeys_debug.h
    IDENTIFIER KCMKEYS
    CATEGORY_NAME org.kde.kcm_keys
    EXPORT KCMKEYS
    DESCRIPTION "System Settings - Global Shortcuts"
)

ecm_qt_install_logging_categories(
    EXPORT KCMKEYS
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)


kcoreaddons_add_plugin(kcm_keys SOURCES ${kcm_keys_SRCS} INSTALL_NAMESPACE "kcms")

target_link_libraries(kcm_keys
    Qt::DBus
    KF5::ConfigGui
    KF5::GlobalAccel
    KF5::I18n
    KF5::KCMUtils
    KF5::KIOWidgets
    KF5::QuickAddons
)

kcoreaddons_desktop_to_json(kcm_keys "kcm_keys.desktop")

install(FILES kcm_keys.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

kpackage_install_package(package kcm_keys kcms)

install(FILES
    schemes/kde3.kksrc
    schemes/kde4.kksrc
    schemes/mac4.kksrc
    schemes/unix3.kksrc
    schemes/win3.kksrc
    schemes/win4.kksrc
    schemes/wm3.kksrc
    DESTINATION  ${KDE_INSTALL_DATADIR}/kcmkeys
)
