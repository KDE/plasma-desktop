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
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kirigami 2.2 as Kirigami
import org.kde.kcm 1.0
import org.kde.private.kcm_cursortheme 1.0
MouseArea {
    id: delegate
    onClicked: {
        view.currentIndex = index;
        view.forceActiveFocus();
    }
    width: view.cellWidth
    height: view.cellHeight
    hoverEnabled: true

    

    Rectangle {
        id: thumbnail
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Kirigami.Units.smallSpacing
        }
        height: width/1.6
        radius: Kirigami.Units.smallSpacing
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window

        color: {
            if (delegate.GridView.isCurrentItem) {
                return Kirigami.Theme.highlightColor;
            } else if (parent.containsMouse) {
                return Qt.tint(Kirigami.Theme.backgroundColor, Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.3))
            } else {
                return Qt.darker(Kirigami.Theme.backgroundColor, 1.1);
            }
        }
        Rectangle {
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing * 2
            }
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            color: Kirigami.Theme.backgroundColor

            PreviewWidget {
                id: previewWidget
                anchors.fill: parent
                themeModel: kcm.cursorsModel
                currentIndex: index
                //The ComboBox component is a disaster
                currentSize: parseInt(sizeCombo.currentText) !== NaN ? parseInt(sizeCombo.currentText) : 0
            }
        }
        ColumnLayout {
            visible: delegate.containsMouse
            anchors.centerIn: parent
            QtControls.ComboBox {
                id: sizeCombo
                Layout.fillWidth: true
                model: kcm.sizesModel
                textRole: "display"
                enabled: kcm.canResize
                onCurrentTextChanged: {
                    kcm.preferredSize = parseInt(sizeCombo.currentText) !== NaN ? parseInt(sizeCombo.currentText) : 0
                }
            }
            QtControls.Button {
                Layout.fillWidth: true
                iconName: "edit-delete"
                text: i18n("Remove &Theme")
                onClicked: kcm.removeTheme(index);
                enabled: kcm.canRemove
            }
        }
    }

    QtControls.Label {
        anchors {
            left: parent.left
            right: parent.right
            top: thumbnail.bottom
        }
        text: model.display
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }
}
