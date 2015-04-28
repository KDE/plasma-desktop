/*
   Copyright (c) 2015 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
//We need units from it
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kcm 1.0

import org.kde.private.kcm_cursortheme 1.0

ColumnLayout {
    implicitWidth: units.gridUnit * 20
    implicitHeight: units.gridUnit * 20

    ConfigModule.quickHelp: i18n("This module lets you configure the mouse cursor theme used.")

    SystemPalette {id: syspal}

    Connections {
        target: kcm
        onSelectedThemeRowChanged: view.currentIndex = kcm.selectedThemeRow;
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        QtControls.ScrollView {
            Rectangle {
                anchors.fill: parent
                z: -1
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            frameVisible: true
            highlightOnFocus: true
            activeFocusOnTab: true
            ListView {
                id: view
                focus: true
                activeFocusOnTab: true
                model: kcm.cursorsModel
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                onCurrentIndexChanged: {
                    kcm.selectedThemeRow = currentIndex;
                    positionViewAtIndex(currentIndex, ListView.Beginning);
                }
                delegate: Delegate {}
                Keys.onPressed: {
                    if (event.key == Qt.Key_Up) {
                        view.decrementCurrentIndex();
                    } else if (event.key == Qt.Key_Down) {
                        view.incrementCurrentIndex();
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.maximumHeight: Layout.minimumHeight
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        QtControls.Button {
            iconName: "document-import"
            text: i18n("&Install From File...")
            onClicked: kcm.installClicked();
            enabled: kcm.canInstall
        }
        QtControls.Button {
            iconName: "get-hot-new-stuff"
            text: i18n("&Get New Theme...")
            onClicked: kcm.getNewClicked();
            enabled: kcm.canInstall
        }
    }
}

