/* SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 */

import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.Page {
    id: previewArea
    implicitWidth: previewWindow.implicitWidth
    implicitHeight: previewWindow.implicitHeight
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    
    actions {
        main: Kirigami.Action {
            iconName: "color-picker"
            text: i18n("Locate Color Role")
            shortcut: "Ctrl+L"
            tooltip: i18n("Locate a color role by clicking on a part of the preview window") + " (" + shortcut + ")"
        }
    }

    PreviewWindow {
        id: previewWindow
        anchors {
            fill: parent
            // Margins are intentionally 2x thicker than the shadows even
            // though it looks like they should be exactly the same as the
            // shadows in the code. Why aren't the shadows as thick as the
            // shadows size? IDK.
            topMargin: previewWindow.shadowSize - shadowYOffset
            leftMargin: previewWindow.shadowSize
            rightMargin: previewWindow.shadowSize
            bottomMargin: previewWindow.shadowSize + shadowYOffset
        }
    }
}
