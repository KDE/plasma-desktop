# SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(backend)

qt_add_dbus_adaptor(kimpanel_dbus_SRCS
    org.kde.impanel.xml
    kimpanelagent.h
    PanelAgent
)

plasma_add_applet(org.kde.plasma.kimpanel
    QML_SOURCES
        qml/ActionMenu.qml
        qml/CandidateHighlight.qml
        qml/ConfigAppearance.qml
        qml/config.qml
        qml/ContextMenu.qml
        qml/InputPanel.qml
        qml/main.qml
        qml/StatusIcon.qml
    CPP_SOURCES
        kimpanel.cpp
        screen.cpp
        kimpanelagent.cpp
        ${kimpanel_dbus_SRCS}
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)

target_compile_definitions(org.kde.plasma.kimpanel PRIVATE -DKIMPANEL_LIBEXEC_DIR=\"${KDE_INSTALL_FULL_LIBEXECDIR}\")
target_link_libraries(org.kde.plasma.kimpanel PRIVATE
    Qt::Gui
    Qt::Qml
    Qt::DBus
)
