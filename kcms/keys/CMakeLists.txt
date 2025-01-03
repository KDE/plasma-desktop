# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_keys\")

# D-Bus interfaces
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

# Shared library
add_library(kglobalaccelmodel SHARED)
target_sources(kglobalaccelmodel PRIVATE
    basemodel.cpp
    filteredmodel.cpp
    globalaccelmodel.cpp
    shortcutsmodel.cpp
    standardshortcutsmodel.cpp
    ${kcm_keys_DBUS_SRCS}
)
generate_export_header(kglobalaccelmodel)
target_include_directories(kglobalaccelmodel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(kglobalaccelmodel
    PUBLIC
        Qt::Core
        Qt::Gui
    PRIVATE
        Qt::DBus
        KF6::ConfigGui
        KF6::GlobalAccel
        KF6::I18n
        KF6::Service
)
set_target_properties(kglobalaccelmodel PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR})

ecm_qt_declare_logging_category(kglobalaccelmodel
    HEADER kcmkeys_debug.h
    IDENTIFIER KCMKEYS
    CATEGORY_NAME org.kde.kcm_keys
    EXPORT KCMKEYS
    DESCRIPTION "System Settings - Global Shortcuts"
)
ecm_qt_install_logging_categories(EXPORT KCMKEYS DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

install(TARGETS kglobalaccelmodel ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

# KCM
kcmutils_add_qml_kcm(kcm_keys)

target_sources(kcm_keys PRIVATE
    kcm_keys.cpp
    keysdata.cpp
    ${kcm_keys_DBUS_SRCS}
)

target_link_libraries(kcm_keys PRIVATE
    Qt::DBus
    Qt::Quick
    KF6::ConfigGui
    KF6::GlobalAccel
    KF6::I18n
    KF6::KCMUtils
    KF6::KIOWidgets
    KF6::KCMUtilsQuick
    kglobalaccelmodel
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
