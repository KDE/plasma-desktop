# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_kded\")

set(kcm_kded_SRCS
    kcmkded.cpp
    modulesmodel.cpp
    filterproxymodel.cpp
    kdedconfigdata.cpp
)

qt_add_dbus_interface(kcm_kded_SRCS ${KDED_DBUS_INTERFACE} kded_interface)

ecm_qt_declare_logging_category(kcm_kded_SRCS HEADER debug.h
                                IDENTIFIER KCM_KDED
                                CATEGORY_NAME kcm_kded
                                DEFAULT_SEVERITY Info
                                DESCRIPTION "KCM for kded services"
                                EXPORT kcm_kded)

ecm_qt_install_logging_categories(
       EXPORT kcm_kded
       DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

kcoreaddons_add_plugin(kcm_kded SOURCES ${kcm_kded_SRCS} INSTALL_NAMESPACE "kcms")

kcoreaddons_desktop_to_json(kcm_kded "kcmkded.desktop")

target_link_libraries(kcm_kded KF5::QuickAddons KF5::Service KF5::I18n KF5::KCMUtils Qt::DBus)

########### install files ###############

install( FILES kcmkded.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )

kpackage_install_package(package kcm5_kded kcms)
