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
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2 as Controls
import QtQuick.Templates 2.2 as T2
import QtGraphicalEffects 1.0

import org.kde.kirigami 2.2 as Kirigami

import org.kde.kcm 1.1 as KCM
import org.kde.private.kcm_cursortheme 1.0

KCM.GridDelegate {
    id: delegate

    text: model.display
    toolTip: model.description

    opacity: model.pendingDeletion ? 0.3 : 1

    thumbnailAvailable: true
    thumbnail: PreviewWidget {
        id: previewWidget
        //for cursor themes we must ignore the native scaling,
        //as they will be rendered by X11/KWin, ignoring whatever Qt
        //scaling
        width: parent.width * Screen.devicePixelRatio
        height: parent.height * Screen.devicePixelRatio
        x: Screen.devicePixelRatio % 1
        y: Screen.devicePixelRatio % 1
        transformOrigin: Item.TopLeft
        scale: 1 / Screen.devicePixelRatio
        themeModel: kcm.cursorsModel
        currentIndex: index
        currentSize: kcm.cursorThemeSettings.cursorSize
    }

    Connections {
        target: kcm
        onThemeApplied: {
            previewWidget.refresh();
        }
    }

    actions: [
        Kirigami.Action {
            iconName: "edit-delete"
            tooltip: i18n("Remove Theme")
            enabled: model.isWritable
            visible: !model.pendingDeletion
            onTriggered: model.pendingDeletion = true
        },
        Kirigami.Action {
            iconName: "edit-undo"
            tooltip: i18n("Restore Cursor Theme")
            visible: model.pendingDeletion
            onTriggered: model.pendingDeletion = false
        }
    ]

    onClicked: {
        view.forceActiveFocus();
        kcm.cursorThemeSettings.cursorTheme = kcm.cursorThemeFromIndex(index);
    }
}
