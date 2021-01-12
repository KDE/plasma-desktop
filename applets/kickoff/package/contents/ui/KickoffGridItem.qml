/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
 *   Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import QtQuick.Layouts 1.12

import "code/tools.js" as Tools

Item {
    id: gridItem

    enabled: !model.disabled

    width: GridView.view.cellWidth
    height: width

    property int itemIndex: model.index
    property url url: model.url || ""
    property var decoration: model.decoration || ""

    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList === true)))

    property alias labelTruncated: label.truncated
    property string display: model.display

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.display

    function openActionMenu(x, y) {
        var actionList = hasActionList ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, GridView.view.model.favoritesModel, model.favoriteId);
        actionMenu.visualParent = gridItem;
        actionMenu.open(x, y);
    }

    function actionTriggered(actionId, actionArgument) {
        var close = (Tools.triggerAction(GridView.view.model, model.index, actionId, actionArgument) === true);

        if (close) {
            plasmoid.expanded = false;
        }
    }
    function activate() {
        gridItem.GridView.view.model.trigger(index, "", null);
        plasmoid.expanded = false;
    }
    Item {
        anchors.fill: parent
        anchors.margins: gridItem.GridView.view.cellMargin
        anchors.topMargin: gridItem.GridView.view.cellMargin + (plasmoid.configuration.gridAllowTwoLines && label.lineCount == 2 ? Math.round(label.height / 2 / 2) : 0)
        Column {
            width: parent.width
            anchors.verticalCenter: parent.verticalCenter
            spacing: Math.round(PlasmaCore.Units.smallSpacing * 1.5) - (plasmoid.configuration.gridAllowTwoLines && label.lineCount == 2 ? Math.round(label.height / 2 / 2) : 0)
            PlasmaCore.IconItem {
                id: icon

                anchors.horizontalCenter: parent.horizontalCenter

                width: iconSize
                height: width

                usesPlasmaTheme: false
                animated: false

                source: model.decoration
            }

            PC3.Label {
                id: label

                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width - PlasmaCore.Units.smallSpacing * 2
                horizontalAlignment: Text.AlignHCenter

                maximumLineCount: plasmoid.configuration.gridAllowTwoLines ? 2 : 1
                elide: Text.ElideRight
                wrapMode: Text.Wrap

                text: ("name" in model ? model.name : model.display)
            }
        }
    }
} // gridItem
