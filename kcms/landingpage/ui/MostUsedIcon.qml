/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami

QQC2.ToolButton {
    // We're using custom properties rather than plain old icon.name: and text:
    // because this would cause the icon and text to be rendered twice with
    // qqc2-desktop-style since it does all its rendering in the background item
    // rather than the contentItem like it should, so overriding the contentItem
    // as we're doing here doesn't completely replace those things as expected.
    property alias kcmIcon: iconItem.source
    property alias kcmName: label.text

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing
    spacing: Kirigami.Units.smallSpacing

    Accessible.name: label.text

    contentItem: RowLayout {
        spacing: parent.spacing

        Kirigami.Icon {
            id: iconItem
            Layout.alignment: Qt.AlignCenter
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: Kirigami.Units.iconSizes.smallMedium
        }

        QQC2.Label {
            id: label
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
}

