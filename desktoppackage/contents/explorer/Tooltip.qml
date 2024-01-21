/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.0 as Layouts
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.20 as Kirigami

MouseArea {
    id: main

    hoverEnabled: true
    onEntered: toolTipHideTimer.running = false
    onExited: toolTipHideTimer.running = true

    width: Kirigami.Units.iconSizes.sizeForLabels * 35
    height: Kirigami.Units.iconSizes.sizeForLabels * 16

    property variant icon
    property string title
    property string description
    property string author
    property string email
    property string license
    property string pluginName
    property bool local

    onClicked: tooltipDialog.visible = false
    Connections {
        target: tooltipDialog
        function onAppletDelegateChanged() {
            if (!tooltipDialog.appletDelegate) {
                return
            }
            icon = tooltipDialog.appletDelegate.icon
            title = tooltipDialog.appletDelegate.title
            description = tooltipDialog.appletDelegate.description
            author = tooltipDialog.appletDelegate.author
            email = tooltipDialog.appletDelegate.email
            license = tooltipDialog.appletDelegate.license
            pluginName = tooltipDialog.appletDelegate.pluginName
            local = tooltipDialog.appletDelegate.local
        }
    }
    Kirigami.Icon {
        id: tooltipIconWidget
        anchors {
            left: parent.left
            top: parent.top
            margins: 8
        }
        width: Kirigami.Units.iconSizes.huge
        height: width
        source: main.icon
    }
    Column {
        id: nameColumn
        spacing: 8
        anchors {
            left: tooltipIconWidget.right
            margins: 8
            top: parent.top
            right: parent.right
        }

        Kirigami.Heading {
            text: title
            level: 2
            anchors.left: parent.left
            anchors.right: parent.right
            height: paintedHeight
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        PlasmaComponents.Label {
            text: description
            textFormat: Text.PlainText
            anchors.left: parent.left
            anchors.right: parent.right
            wrapMode: Text.Wrap
        }
    }
    Layouts.GridLayout {
        columns: 2
        anchors {
            top: (nameColumn.height > tooltipIconWidget.height) ? nameColumn.bottom : tooltipIconWidget.bottom
            topMargin: 16
            horizontalCenter: parent.horizontalCenter
        }
        PlasmaComponents.Label {
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "License:")
            textFormat: Text.PlainText
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        PlasmaComponents.Label {
            id: licenseText
            text: license
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        PlasmaComponents.Label {
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Author:")
            textFormat: Text.PlainText
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        PlasmaComponents.Label {
            text: author
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        PlasmaComponents.Label {
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Email:")
            textFormat: Text.PlainText
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        PlasmaComponents.Label {
            text: email
            textFormat: Text.PlainText
        }
    }

    PlasmaComponents.Button {
        id: uninstallButton
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        opacity: local ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: Kirigami.Units.longDuration }
        }
        iconSource: "application-exit"
        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Uninstall")
        onClicked: {
            widgetExplorer.uninstall(pluginName)
            tooltipDialog.visible = false
        }
    }
}
