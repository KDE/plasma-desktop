set(kcm_users_SRCS
    kcm.cpp
    user.cpp
    usermodel.cpp
)

set_source_files_properties(org.freedesktop.Accounts.xml
                        PROPERTIES NO_NAMESPACE TRUE)

set_source_files_properties(org.freedesktop.Accounts.User.xml
                        PROPERTIES NO_NAMESPACE TRUE)

qt_add_dbus_interface(kcm_users_SRCS
    org.freedesktop.Accounts.xml
    accounts_interface
)

qt_add_dbus_interface(kcm_users_SRCS
    org.freedesktop.Accounts.User.xml
    user_interface
)

set(login1_manager_xml org.freedesktop.login1.Manager.xml)
set_source_files_properties(${login1_manager_xml} PROPERTIES INCLUDE "usersessions.h")
qt_add_dbus_interface(kcm_users_SRCS
    ${login1_manager_xml}
    login1_interface
)

ecm_qt_declare_logging_category(kcm_users_SRCS
    HEADER kcmusers_debug.h
    IDENTIFIER KCMUSERS
    CATEGORY_NAME org.kde.kcm_users
    EXPORT KCMUSERS
    DESCRIPTION "System Settings - Users"
)

ecm_qt_install_logging_categories(
    EXPORT KCMUSERS
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

kcoreaddons_add_plugin(kcm_users SOURCES ${kcm_users_SRCS} INSTALL_NAMESPACE "kcms")

target_link_libraries(kcm_users
    Qt::Quick
    Qt::QuickWidgets
    Qt::Concurrent
    KF5::CoreAddons
    KF5::Declarative
    KF5::I18n
    KF5::QuickAddons
    KF5::Wallet
    Qt::DBus
    crypt
)

kcoreaddons_desktop_to_json(kcm_users "../kcm_users.desktop" SERVICE_TYPES kcmodule.desktop)
