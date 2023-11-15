# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

ecm_qt_declare_logging_category(kwindevices_SRCS
    HEADER
        logging.h
    IDENTIFIER
        LIBKWINDEVICES
    CATEGORY_NAME
        org.kde.libkwindevices
    DEFAULT_SEVERITY
        Critical
    DESCRIPTION
        "Library for kwin devices"
    EXPORT
        libkwindevices
)

qt_add_dbus_interface(kwindevices_SRCS "${KWIN_INPUTDEVICE_INTERFACE}" InputDevice_interface)

add_library(kwindevices STATIC ${kwindevices_SRCS} devicesmodel.cpp inputdevice.cpp)
set_property(TARGET kwindevices PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kwindevices
    Qt::Core
    Qt::DBus
    Qt::Gui
)
