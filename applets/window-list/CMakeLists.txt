# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

plasma_add_applet(org.kde.plasma.windowlist
    QML_SOURCES
        ConfigGeneral.qml
        config.qml
        main.qml
        MenuButton.qml
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)
