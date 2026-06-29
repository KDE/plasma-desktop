/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3

ItemAbstractDelegate {
    id: root

    property alias iconSize: icon.implicitWidth
    property int itemIndex: root.index
    property var m: model
    property bool showUnfavoritePlaceholder: false
    property bool isDraggableFavorite: false

    width: GridView.view.cellWidth
    height: width

    enabled: !root.disabled
    favoritesModel: GridView.view.model.favoritesModel
    baseModel: GridView.view.model
    dragActive: dragHandler.active

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.display ?? ""

    DragHandler {
        id: dragHandler
        target: null
        onActiveChanged: if (active) {
            root.contentItem.grabToImage(function(result) {
                root.Drag.imageSource = result.url
                root.Drag.active = true
            })
        } else {
            root.Drag.active = false
        }
    }
    Drag.dragType: Drag.Automatic
    Drag.mimeData: root.isDraggableFavorite ? {
        'favoritedrag': '',
        "text/uri-list" : [root.url]
    } : {
        "text/uri-list" : [root.url]
    }

    background.visible: false // we want the default background's spacing, but not the base color
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        visible: !root.showUnfavoritePlaceholder

        Kirigami.Icon {
            id: icon

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            implicitHeight: width

            animated: false

            source: root.model.decoration

            Loader {
                anchors {
                    right: parent.right
                    rightMargin: -root.rightPadding
                    top: parent.top
                }
                visible: active
                active: root.isNewlyInstalled ?? false

                sourceComponent: Kirigami.Badge {
                    text: i18nc("@label Newly-installed app, badge, keep short", "New!")
                    type: Kirigami.Badge.Type.Positive
                    Accessible.name: i18nc("@label Accessible name for badge", "Newly-installed application")
                }
            }
        }

        PlasmaComponents3.Label {
            id: label

            Layout.fillWidth: true
            Layout.preferredHeight: label.lineCount === 1 ? label.implicitHeight * 2 : label.implicitHeight

            horizontalAlignment: Text.AlignHCenter

            maximumLineCount: 2
            elide: Text.ElideMiddle
            wrapMode: Text.Wrap

            text: root.model.display ?? ""
            textFormat: Text.PlainText
        }
    }

    Loader {
        active: root.showUnfavoritePlaceholder
        anchors.fill: parent

        sourceComponent: Item {
            KSvg.FrameSvgItem {
                anchors.fill: parent

                imagePath: "widgets/viewitem"
                prefix: "selected"

                opacity: 0.5

                Kirigami.Icon {
                    anchors.centerIn: parent

                    width: root.iconSize
                    height: width

                    source: "list-remove"
                    active: false
                }
            }
        }
    }

    PlasmaComponents3.ToolTip {
        text: root.model.description ?? ""
        visible: root.hovered && !ActionMenu.opened
    }
}
