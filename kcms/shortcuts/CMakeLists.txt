# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_shortcuts\")

# KCM
kcmutils_add_qml_kcm(kcm_shortcuts)

# KF6
find_package(KF6 REQUIRED COMPONENTS ItemModels)

# logging
ecm_qt_declare_logging_category(kcm_shortcuts
    HEADER kcm_shortcuts_debug.h
    IDENTIFIER KCMSHORTCUTS
    CATEGORY_NAME org.kde.kcm_shortcuts
    EXPORT KCMSHORTCUTS
    DESCRIPTION "System Settings - Shortcuts and Gestures"
)
ecm_qt_install_logging_categories(EXPORT KCMSHORTCUTS DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

target_sources(kcm_shortcuts PRIVATE
    kcm_shortcuts.cpp
    shortcutslistmodel.cpp   
)

target_link_libraries(kcm_shortcuts PRIVATE
    Qt::DBus
    Qt::Quick
    KF6::ConfigGui
    KF6::GlobalAccel
    KF6::I18n
    KF6::ItemModels
    KF6::KCMUtils
    KF6::KIOWidgets
    KF6::KCMUtilsQuick
    kglobalaccelmodel
)
