/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore

QQC2.Button {
    id: button

    property string kcmId
    // We're using custom properties rather than plain old icon.name: and text:
    // because this would cause the icon and text to be rendered twice with
    // qqc2-desktop-style since it does all its rendering in the background item
    // rather than the contentItem like it should, so overriding the contentItem
    // as we're doing here doesn't completely replace those things as expected.
    property alias kcmIcon: iconItem.source
    property alias kcmName: label.text

    readonly property PlasmaCore.Action kcmAction: kcm.kcmAction(kcmId)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    topPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing
    spacing: Kirigami.Units.smallSpacing

    Accessible.name: label.text

    onClicked: {
        if (kcmAction) {
            kcmAction.trigger();
        }
    }

    contentItem: RowLayout {
        spacing: parent.spacing

        Kirigami.Icon {
            id: iconItem
            Layout.alignment: Qt.AlignCenter
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small
            source: button.kcmAction?.icon ?? ""
        }

        QQC2.Label {
            id: label
            Layout.fillWidth: true
            elide: Text.ElideRight
            textFormat: Text.PlainText
            text: button.kcmAction?.text ?? ""
        }
    }
}

