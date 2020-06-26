/* SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 */

import QtQuick 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13 as QQC2
import org.kde.kirigami 2.13 as Kirigami

Kirigami.Page {
    id: root
    background: Rectangle {color: "transparent"}
    //TODO: use real colorscheme name
    title: i18n("Breeze Dark")
    
    actions {
        main: Kirigami.Action {
            iconName: "document-save"
            text: i18n("Action 1")
            shortcut: "Ctrl+S"
//             tooltip: 
        }
        left: Kirigami.Action {
            iconName: "edit-undo"
            text: i18n("Action 2")
            shortcut: "Ctrl+Z"
//             tooltip: root.fakeControlToolTip
        }
        right: Kirigami.Action {
            iconName: "edit-redo"
            text: i18n("Action 3")
            shortcut: "Ctrl+Shift+Z"
//             tooltip: root.fakeControlToolTip
        }
    }
}
