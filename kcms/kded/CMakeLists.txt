# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_kded\")

set(kcm_kded_SRCS
    kcmkded.cpp
    modulesmodel.cpp
    filterproxymodel.cpp
)

qt5_add_dbus_interface(kcm_kded_SRCS ${KDED_DBUS_INTERFACE} kded_interface)

ecm_qt_declare_logging_category(kcm_kded_SRCS HEADER debug.h
                                IDENTIFIER KCM_KDED
                                CATEGORY_NAME kcm_kded
                                DEFAULT_SEVERITY Info)

add_library(kcm_kded MODULE ${kcm_kded_SRCS})

kcoreaddons_desktop_to_json(kcm_kded "kcmkded.desktop")

target_link_libraries(kcm_kded KF5::QuickAddons KF5::Service KF5::I18n Qt5::DBus)

install(TARGETS kcm_kded  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms )

########### install files ###############

install( FILES kcmkded.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )

kpackage_install_package(package kcm5_kded kcms)
