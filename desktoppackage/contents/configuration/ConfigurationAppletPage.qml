/*
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.kirigami 2.10 as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: configItem.name

    required property var configItem

    signal settingValueChanged()

    function saveConfig() {
        for (let key in plasmoid.configuration) {
            if (loader.item["cfg_" + key] != undefined) {
                plasmoid.configuration[key] = loader.item["cfg_" + key]
            }
        }

        // For ConfigurationContainmentActions.qml
        if (loader.item.hasOwnProperty("saveConfig")) {
            loader.item.saveConfig()
        }
    }

    implicitHeight: loader.height

    padding: Kirigami.Units.largeSpacing
    bottomPadding: 0

    Loader {
        id: loader
        width: parent.width
        // HACK the height of the loader is based on the implicitHeight of the content.
        // Unfortunately not all content items have a sensible implicitHeight.
        // If it is zero fall back to the height of its children
        // Also make it at least as high as the page itself. Some existing configs assume they fill the whole space
        // TODO KF6 clean this up by making all configs based on SimpleKCM/ScrollViewKCM/GridViewKCM
        height: Math.max(root.availableHeight, item.implicitHeight ? item.implicitHeight : item.childrenRect.height)

        Component.onCompleted: {
            const plasmoidConfig = plasmoid.configuration

            const props = {}
            for (let key in plasmoidConfig) {
                props["cfg_" + key] = plasmoid.configuration[key]
            }

            setSource(configItem.source, props)
        }

        onLoaded: {
            const plasmoidConfig = plasmoid.configuration;

            for (let key in plasmoidConfig) {
                const changedSignal = item["cfg_" + key + "Changed"]
                if (changedSignal) {
                    changedSignal.connect(root.settingValueChanged)
                }
            }

            const configurationChangedSignal = item.configurationChanged
            if (configurationChangedSignal) {
                configurationChangedSignal.connect(root.settingValueChanged)
            }
        }
    }
}
