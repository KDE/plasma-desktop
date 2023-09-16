# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kded\")

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

kcmutils_add_qml_kcm(kcm_kded SOURCES ${kcm_kded_SRCS})
target_link_libraries(kcm_kded PRIVATE KF6::KCMUtilsQuick KF6::I18n Qt::DBus)
