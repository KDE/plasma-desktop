# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN="kcm_gamecontroller")

kcmutils_add_qml_kcm(kcm_gamecontroller INSTALL_NAMESPACE "plasma/kcms/systemsettings")

ecm_qt_declare_logging_category(kcm_gamecontroller
    HEADER
        logging.h
    IDENTIFIER
        KCM_GAMECONTROLLER
    CATEGORY_NAME
        kcm_gamecontroller
    DEFAULT_SEVERITY
        Critical
    DESCRIPTION
        "KCM for game controller"
    EXPORT
        kcm_gamecontroller
)

ecm_qt_install_logging_categories(
    EXPORT kcm_gamecontroller
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

target_sources(kcm_gamecontroller PRIVATE
    devicemodel.cpp
    gamepad.cpp
    axesmodel.cpp
    buttonmodel.cpp
    kcm.cpp
)

target_link_libraries(kcm_gamecontroller PRIVATE
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::KCMUtilsQuick
    KF6::I18n
    Qt6::Quick
    Qt6::QuickWidgets
    SDL2::SDL2
)
