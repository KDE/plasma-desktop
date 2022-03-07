/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.4
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.6 as Kirigami

import org.kde.plasma.plasmoid 2.0

/**
 * A copy of Kirigami.AboutPage adapted to KPluginMetadata instead of KAboutData
 */
Kirigami.ScrollablePage {
    id: page
    title: i18n("About")

    Component {
        id: personDelegate

        RowLayout {
            height: implicitHeight + (Kirigami.Units.smallSpacing * 2)

            spacing: Kirigami.Units.smallSpacing * 2
            Kirigami.Icon {
                width: Kirigami.Units.iconSizes.smallMedium
                height: width
                source: "user"
            }
            QQC2.Label {
                text: modelData.name
            }
            Row {
                // Group action buttons together
                spacing: 0
                QQC2.ToolButton {
                    visible: modelData.emailAddress
                    width: height
                    icon.name: "mail-sent"
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Send an email to %1", modelData.emailAddress)
                    onClicked: Qt.openUrlExternally("mailto:%1".arg(modelData.emailAddress))
                }
                QQC2.ToolButton {
                    visible: modelData.webAddress
                    width: height
                    icon.name: "globe"
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: modelData.webAddress
                    onClicked: Qt.openUrlExternally(modelData.webAddress)
                }
            }
        }
    }

    Kirigami.FormLayout {
        id: form
        GridLayout {
            columns: 2
            Layout.fillWidth: true

            Kirigami.Icon {
                Layout.rowSpan: 2
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                Layout.preferredWidth: height
                Layout.maximumWidth: page.width / 3;
                Layout.rightMargin: Kirigami.Units.largeSpacing
                source: Plasmoid.metaData.iconName || Plasmoid.metaData.pluginId
                fallback: "application-x-plasma"
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                text: Plasmoid.metaData.name + " " + Plasmoid.metaData.version
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                level: 2
                wrapMode: Text.WordWrap
                text: Plasmoid.metaData.description
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Kirigami.Heading {
            Kirigami.FormData.isSection: true
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Copyright")
        }
        QQC2.Label {
            Layout.leftMargin: Kirigami.Units.gridUnit
            text: Plasmoid.metaData.extraInformation
            visible: text.length > 0
        }
        QQC2.Label {
            Layout.leftMargin: Kirigami.Units.gridUnit
            text: Plasmoid.metaData.copyrightText
            visible: text.length > 0
        }
        Kirigami.UrlButton {
            Layout.leftMargin: Kirigami.Units.gridUnit
            url: Plasmoid.metaData.website
            visible: url.length > 0
        }

        RowLayout {
            Layout.leftMargin: Kirigami.Units.smallSpacing
            QQC2.Label { text: i18nd("plasma_shell_org.kde.plasma.desktop", "License:") }
            Kirigami.LinkButton {
                text: Plasmoid.metaData.license
                onClicked: {
                    licenseSheet.text = Plasmoid.metaData.licenseText
                    licenseSheet.title = Plasmoid.metaData.license
                    licenseSheet.open()
                }
            }
        }
        Kirigami.Heading {
            Layout.fillWidth: true
            Kirigami.FormData.isSection: visible
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Authors")
            visible: Plasmoid.metaData.authors.length > 0
        }
        Repeater {
            model: Plasmoid.metaData.authors
            delegate: personDelegate
        }
        Kirigami.Heading {
            height: visible ? implicitHeight : 0
            Kirigami.FormData.isSection: visible
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Credits")
            visible: repCredits.count > 0
        }
        Repeater {
            id: repCredits
            model: Plasmoid.metaData.otherContributors
            delegate: personDelegate
        }
        Kirigami.Heading {
            height: visible ? implicitHeight : 0
            Kirigami.FormData.isSection: visible
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Translators")
            visible: repTranslators.count > 0
        }
        Repeater {
            id: repTranslators
            model: Plasmoid.metaData.translators
            delegate: personDelegate
        }
    }
    QQC2.Dialog {
        id: licenseSheet
        property alias text: licenseLabel.text

        width: 0.75 * parent.width
        height: 0.75 * parent.height

        x: Math.round((parent.width - width) / 2)
        y: Kirigami.Units.smallSpacing

        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0
        topPadding: Kirigami.Units.smallSpacing
        topInset: Kirigami.Units.smallSpacing

        contentItem: QQC2.ScrollView {
            id: scroll
            Component.onCompleted: background.visible = true
            Flickable {
                id: flickable
                contentWidth: width
                contentHeight: licenseLabel.contentHeight
                clip: true
                QQC2.Label {
                    id: licenseLabel
                    width: parent.width
                    x: Math.max(0, (width - contentWidth)/2)
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}

