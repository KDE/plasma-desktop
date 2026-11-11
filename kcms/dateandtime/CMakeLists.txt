# SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_clock\")

kcmutils_add_qml_kcm(kcm_clock
    SOURCES
        dateandtime.cpp
        dateandtime.h
        message.h
)

ecm_qt_declare_logging_category(kcm_clock
    HEADER datetime_logging.h
    IDENTIFIER KCM_CLOCK
    CATEGORY_NAME kcm_clock
    DESCRIPTION "KCM for date and time"
    EXPORT KCM_CLOCK
)

qt_add_dbus_interface(kcm_clock_DBUS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/timedated1.xml timedated_interface)
target_sources(kcm_clock PRIVATE ${kcm_clock_DBUS_SRCS})

target_link_libraries(kcm_clock PRIVATE
    Qt::DBus
    Qt::Quick
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::I18n
)
