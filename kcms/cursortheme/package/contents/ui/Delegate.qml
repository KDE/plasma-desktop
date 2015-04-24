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
//We need units from it
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kcm 1.0
import org.kde.private.kcm_cursortheme 1.0
MouseArea {
    id: delegate
    onClicked: {
        view.currentIndex = index;
        view.forceActiveFocus();
    }
    width: view.width
    height: delegateLayout.height + units.largeSpacing * 2
    hoverEnabled: true

    Rectangle {
        anchors.fill: parent
        opacity: delegate.ListView.isCurrentItem ? 1 : 0.4
        color: {
            if (delegate.ListView.isCurrentItem || parent.containsMouse) {
                return syspal.highlight;
            } else if (index % 2 == 0) {
                return syspal.base;
            } else {
                return syspal.alternateBase;
            }
        }
    }
    ColumnLayout {
        id: delegateLayout
        anchors.centerIn: parent
        width: view.width - units.largeSpacing * 2
        //TODO: an Header component
        QtControls.Label {
            Layout.fillWidth: true
            text: model.display
            color: delegate.ListView.isCurrentItem ? syspal.highlightedText : syspal.text
            font.pointSize: commentLabel.font.pointSize * 1.5
        }
        PreviewWidget {
            id: previewWidget
            Layout.minimumWidth: implicitWidth
            Layout.minimumHeight: implicitHeight
            Layout.maximumWidth: Layout.minimumWidth
            Layout.maximumHeight: Layout.minimumHeight
            themeModel: kcm.cursorsModel
            currentIndex: index
            //The ComboBox component is a disaster
            currentSize: parseInt(sizeCombo.currentText) !== NaN ? parseInt(sizeCombo.currentText) : 0
        }

        QtControls.Label {
            id: commentLabel
            Layout.fillWidth: true
            text: model.description
            wrapMode: Text.WordWrap
            color: delegate.ListView.isCurrentItem ? syspal.highlightedText : syspal.text
        }


    }
}
