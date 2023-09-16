# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_keys\")

kcmutils_add_qml_kcm(kcm_keys)

target_sources(kcm_keys PRIVATE
    basemodel.cpp
    kcm_keys.cpp
    filteredmodel.cpp
    globalaccelmodel.cpp
    shortcutsmodel.cpp
    standardshortcutsmodel.cpp
    keysdata.cpp
)

set(kglobalaccel_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf6_org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES
    INCLUDE "kglobalaccel.h"
    NO_NAMESPACE TRUE
    CLASSNAME "KGlobalAccelInterface"
)
qt_add_dbus_interface(kcm_keys_DBUS_SRCS ${kglobalaccel_xml} kglobalaccel_interface)

set(kglobalaccel_component_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf6_org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES
    INCLUDE "kglobalaccel.h"
    NO_NAMESPACE TRUE
    CLASSNAME "KGlobalAccelComponentInterface"
)
qt_add_dbus_interface(kcm_keys_DBUS_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface)

target_sources(kcm_keys PRIVATE ${kcm_keys_DBUS_SRCS})

ecm_qt_declare_logging_category(kcm_keys
    HEADER kcmkeys_debug.h
    IDENTIFIER KCMKEYS
    CATEGORY_NAME org.kde.kcm_keys
    EXPORT KCMKEYS
    DESCRIPTION "System Settings - Global Shortcuts"
)

ecm_qt_install_logging_categories(EXPORT KCMKEYS DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

target_link_libraries(kcm_keys PRIVATE
    Qt::DBus
    Qt::Quick
    KF6::ConfigGui
    KF6::GlobalAccel
    KF6::I18n
    KF6::KCMUtils
    KF6::KIOWidgets
    KF6::KCMUtilsQuick
)

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
