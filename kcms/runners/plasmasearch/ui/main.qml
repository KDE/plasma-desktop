/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.1

import org.kde.config
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils
import org.kde.newstuff 1.91 as NewStuff

KCMUtils.ScrollViewKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 18

    actions: [
        Kirigami.Action {
            icon.name: "configure"
            text: i18n("Configure KRunner…")
            onTriggered: kcm.showKRunnerKCM()
        },
        NewStuff.Action {
            text: i18n("Get New Plugins…")
            visible: KAuthorized.authorize(KAuthorized.GHNS)
            configFile: "krunner.knsrc"
            onEntryEvent: (entry, event) => {
                if (event === NewStuff.Engine.StatusChangedEvent) {
                    kcm.reloadPlugins()
                }
            }
        }
    ]

    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18n("Enable or disable plugins (used in KRunner, Application Launcher, and the Overview effect). Mark plugins as favorites and arrange them in order you want to see them.")
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
        }
    }

    view: KCMUtils.PluginSelector {
        id: pluginSelector
        sourceModel: kcm.model
        query: searchField.text
        reuseItems: true // delegates are fairly expensive to construct due to their use of ActionToolBar
        delegate: Item { // Needed to avoid visual glitches in ListItemDragHandle
            id: delegateItem
            width: pluginSelector.width
            implicitHeight: pluginDelegate.implicitHeight
            // PluginDelegate must either be a direct delegate, or have a model explicitly set
            // "model: model" would just be a binding loop, so we store the model in a property of the
            // direct delegate Item and access it from PluginDelegate
            property var modelObject: model
            property bool isFavorite: model.category === kcm.favoriteCategory
            KCMUtils.PluginDelegate {
                id: pluginDelegate
                model: delegateItem.modelObject
                width: pluginSelector.width
                property var drag: Kirigami.ListItemDragHandle {
                    property int dropNewIndex
                    listItem: pluginDelegate
                    listView: pluginSelector
                        // Moving the rows now would cause a model reset and we could not drag an entry multiple rows up/down
                    onMoveRequested: (oldIndex, newIndex) => (dropNewIndex = newIndex)
                    onDropped: () => kcm.movePlugin(model.metaData, dropNewIndex)
                }
                Component.onCompleted: {
                    if (isFavorite) {
                        leading = drag
                    }
                }
                additionalActions: [
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.IconOnly
                        text: isFavorite ? i18n("Remove from favorites") : i18n("Add to favorites")
                        icon.name: isFavorite ? "starred-symbolic": "non-starred-symbolic"
                        onTriggered: isFavorite ? kcm.removeFromFavorites(model.metaData) : kcm.addToFavorites(model.metaData)
                    }
                ]
                onConfigTriggered: kcm.showKCM(model.config, [], model.metaData)
                highlighted: false
            }
        }
    }
}
