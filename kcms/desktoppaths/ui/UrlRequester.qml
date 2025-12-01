/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

Row {
    id: urlRequester

    required property url location
    required property url defaultLocation
    required property int textFieldWidth

    readonly property int implicitTextFieldWidth: metrics.width + textField.leftPadding + textField.rightPadding

    /**
     * Emitted when the user selects a new folder
     */
    signal newLocationSelected(url newLocation)

    spacing: Kirigami.Units.smallSpacing

    TextMetrics {
        id: metrics
        text: textField.text
    }

    Component {
        id: fileDialogComponent

        FolderDialog {
            id: fileDialog
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]

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
        width: urlRequester.textFieldWidth
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
