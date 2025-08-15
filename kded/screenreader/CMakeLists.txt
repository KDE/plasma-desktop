# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(screenreader INSTALL_NAMESPACE "kf6/kded")

target_sources(screenreader PRIVATE screenreader.cpp)

kconfig_target_kcfg_file(screenreader
    GENERATE_MOC
    FILE kcmaccessibilityscreenreader.kcfg
    HEADER kcmaccessibilityscreenreader.h
    CLASS_NAME ScreenReaderSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

ecm_qt_declare_logging_category(screenreader
    HEADER
        logging.h
    IDENTIFIER
        KDED_SCREENREADER
    CATEGORY_NAME
        kded_screenreader
    DESCRIPTION
        "Screenreader KDED Module"
    EXPORT
        screenreader
)

ecm_qt_install_logging_categories(
    EXPORT screenreader
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

target_link_libraries(screenreader PRIVATE KF6::CoreAddons KF6::DBusAddons KF6::ConfigGui)

install(FILES org.kde.screenreader.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)
install(FILES kcmaccessibilityscreenreader.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
