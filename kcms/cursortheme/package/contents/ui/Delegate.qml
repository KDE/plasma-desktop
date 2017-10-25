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
import QtQuick.Controls 2.2 as Controls
import QtQuick.Templates 2.2 as T2
import QtGraphicalEffects 1.0

import org.kde.kirigami 2.2 as Kirigami

import org.kde.kcm 1.0
import org.kde.private.kcm_cursortheme 1.0

GridDelegate {
    id: delegate

    text: model.display
    toolTip: model.description

    thumbnail: PreviewWidget {
        id: previewWidget
        anchors.fill: parent
        themeModel: kcm.cursorsModel
        currentIndex: index
        //The ComboBox component is a disaster
        currentSize: parseInt(sizeCombo.currentText) !== NaN ? parseInt(sizeCombo.currentText) : 0
    }

    actionButtons: [
        Controls.ToolButton {
            Kirigami.Icon {
                anchors.centerIn: parent
                width: Kirigami.Units.iconSizes.smallMedium
                height: width
                source: "edit-delete"
            }
            onClicked: kcm.removeTheme(index);
            enabled: model.isWritable
            Controls.ToolTip.delay: 1000
            Controls.ToolTip.timeout: 5000
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.text: i18n("Remove Theme")
        }
    ]

    onClicked: {
        view.currentIndex = index;
        view.forceActiveFocus();
    }
}
