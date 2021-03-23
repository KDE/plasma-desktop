/*
 *  Copyright 2020 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0

import org.kde.kirigami 2.10 as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: configItem.name

    required property var configItem

    property Item loadedItem: null

    signal settingValueChanged()

    function saveConfig() {
        for (let key in plasmoid.configuration) {
            if (root.loadedItem["cfg_" + key] != undefined) {
                plasmoid.configuration[key] = root.loadedItem["cfg_" + key]
            }
        }

        // For ConfigurationContainmentActions.qml
        if (root.loadedItem.hasOwnProperty("saveConfig")) {
            root.loadedItem.saveConfig()
        }
    }

    // We wrap this in a QtObject so that we can run some code before the
    // ScrollablePage completes
    QtObject {
        // Legacy wrapper item to allow older things to work well
        property Component item: Component {
            id: legacyWrapper

            Item {
                property Item item: null

                width: root.width
                height: Math.max(root.availableHeight, this.item.implicitHeight ? this.item.implicitHeight : this.item.childrenRect.height)

                implicitWidth: item.implicitWidth
                implicitHeight: item.implicitHeight
            }
        }

        Component.onCompleted: {
            const component = Qt.createComponent(configItem.source)

            const plasmoidConfig = plasmoid.configuration
            const props = {}
            for (let key in plasmoidConfig) {
                props["cfg_" + key] = plasmoid.configuration[key]
            }

            let item = component.createObject(root, props)
            root.loadedItem = item
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

            if (!(item instanceof ListView || item instanceof GridView)) {
                let wrapper = legacyWrapper.createObject(root, {"item": item})
                item.parent = wrapper
                item.anchors.fill = wrapper
                root.mainItem = wrapper
            } else {
                root.mainItem = item
            }

            if ("pageHeader" in item && item.pageHeader instanceof Component) {
                let built = item.pageHeader.createObject(root, {})
                root.header = built
            } else if ("pageFooter" in item && item.pageFooter instanceof Component) {
                let built = item.pageFooter.createObject(root, {})
                root.footer = built
            }
        }
    }
}
