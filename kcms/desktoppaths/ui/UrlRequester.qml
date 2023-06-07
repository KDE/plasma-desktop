/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 6.3
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

Row {
    id: urlRequester

    required property url location
    required property url defaultLocation

    /**
     * Emitted when the user selects a new folder
     */
    signal newLocationSelected(url newLocation)

    spacing: Kirigami.Units.smallSpacing

    Component {
        id: fileDialogComponent

        FolderDialog {
            id: fileDialog
            currentFolder: shortcuts.home

            onAccepted: {
                urlRequester.newLocationSelected(selectedFolder);
                destroy();
            }
            onRejected: destroy()
            Component.onCompleted: open()
        }
    }

    KCM.SettingHighlighter {
        highlight: location !== defaultLocation
    }

    QQC2.TextField {
        id: textField
        width: Kirigami.Units.gridUnit * 10
        height: Math.max(fileDialogButton.implicitHeight, textField.implicitHeight)
        text: location.toString().substr(7)
        readOnly: true

        Accessible.description: urlRequester.Accessible.description
        QQC2.ToolTip.text: urlRequester.Accessible.description
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.visible: textField.hovered
    }

    QQC2.Button {
        id: fileDialogButton
        anchors.verticalCenter: textField.verticalCenter

        display: QQC2.AbstractButton.IconOnly
        icon.name: "document-open"
        text: i18nc("@action:button", "Choose new location")

        Accessible.description: text
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.visible: fileDialogButton.hovered

        onClicked: fileDialogComponent.incubateObject(urlRequester)
    }
}
