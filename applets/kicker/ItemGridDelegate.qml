/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3

ItemAbstractDelegate {
    id: item

    property alias iconSize: icon.implicitWidth
    property bool showLabel: true
    property int itemIndex: item.index
    property var m: model

    width: GridView.view.cellWidth
    height: width

    enabled: !item.disabled
    favoritesModel: GridView.view.model.favoritesModel
    baseModel: GridView.view.model

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.display ?? ""

    Keys.onReturnPressed: activate()
    Keys.onEnterPressed: activate()

    function activate(): void {
        item.baseModel.trigger(index, "", null);
        item.interactionConcluded()
    }

    TapHandler {
        // dedicated tapHandler as ItemDelegate's clicked conflicts with DragHandler
        id: tapHandler
        onTapped: activate()
    }

    DragHandler {
        id: dragHandler
        target: null
        onActiveChanged: if (active) {
            item.contentItem.grabToImage(function(result) {
                item.Drag.imageSource = result.url
                item.Drag.active = true
            })
        } else {
            item.Drag.active = false
        }
    }
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "text/uri-list" : [item.url]
    }

    background.visible: false // we want the default background's spacing, but not the base color
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            id: icon

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            implicitHeight: width

            animated: false

            source: item.decoration
        }

        PlasmaComponents3.Label {
            id: label

            visible: item.showLabel
            Layout.fillWidth: true
            Layout.preferredHeight: label.lineCount === 1 ? label.implicitHeight * 2 : label.implicitHeight

            horizontalAlignment: Text.AlignHCenter

            maximumLineCount: 2
            elide: Text.ElideMiddle
            wrapMode: Text.Wrap

            text: item.model.display ?? ""
            textFormat: Text.PlainText
        }
    }

    PlasmaComponents3.ToolTip {
        text: item.model.description ?? ""
        visible: item.hovered
    }
}
