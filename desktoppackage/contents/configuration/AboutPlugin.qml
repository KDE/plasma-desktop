/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.3

import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

/**
 * A copy of Kirigami.AboutPage adapted to KPluginMetadata instead of KAboutData
 */
KCM.SimpleKCM {
    id: page
    title: i18n("About")

    property var metaData: Plasmoid.metaData

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
                textFormat: Text.PlainText
            }
            Row {
                // Group action buttons together
                spacing: 0
                QQC2.ToolButton {
                    visible: modelData.emailAddress
                    width: height
                    icon.name: "mail-sent"

                    display: QQC2.AbstractButton.IconOnly
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Send an email to %1", modelData.emailAddress)

                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text

                    onClicked: Qt.openUrlExternally("mailto:%1".arg(modelData.emailAddress))
                }
                QQC2.ToolButton {
                    visible: modelData.webAddress
                    width: height
                    icon.name: "globe"

                    display: QQC2.AbstractButton.IconOnly
                    text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info:tooltip %1 url", "Open website %1", modelData.webAddress)

                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: modelData.webAddress

                    onClicked: Qt.openUrlExternally(modelData.webAddress)
                }
            }
        }
    }

    Component {
        id: licenseComponent

        Kirigami.OverlaySheet {
            property alias text: licenseLabel.text

            onClosed: destroy()

            Kirigami.SelectableLabel {
                id: licenseLabel
                implicitWidth: Math.max(Kirigami.Units.gridUnit * 25, Math.round(page.width / 2), contentWidth)
                wrapMode: Text.WordWrap
            }

            Component.onCompleted: open();
        }
    }

    Item {
        height: childrenRect.height

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Kirigami.Units.largeSpacing

            GridLayout {
                columns: 2
                Layout.fillWidth: true

                Kirigami.Icon {
                    Layout.rowSpan: 2
                    Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                    Layout.preferredWidth: height
                    Layout.maximumWidth: page.width / 3;
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    source: page.metaData.iconName || page.metaData.pluginId
                    fallback: "application-x-plasma"
                }

                Kirigami.Heading {
                    Layout.fillWidth: true
                    text: page.metaData.name + " " + page.metaData.version
                    textFormat: Text.PlainText
                }

                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 15
                    level: 2
                    wrapMode: Text.WordWrap
                    text: page.metaData.description
                    textFormat: Text.PlainText
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }

            Kirigami.Heading {
                Layout.topMargin: Kirigami.Units.smallSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Copyright")
                textFormat: Text.PlainText
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.leftMargin: Kirigami.Units.smallSpacing

                QQC2.Label {
                    text: page.metaData.copyrightText
                    textFormat: Text.PlainText
                    visible: text.length > 0
                }
                Kirigami.UrlButton {
                    url: page.metaData.website
                    visible: url.length > 0
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "License:")
                        textFormat: Text.PlainText
                    }
                    Kirigami.LinkButton {
                        text: page.metaData.license
                        Accessible.description: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info:whatsthis", "View license text")
                        onClicked: {
                            licenseComponent.incubateObject(page.Window.window.contentItem, {
                                "text": page.metaData.licenseText,
                                "title": page.metaData.license,
                            }, Qt.Asynchronous);
                        }
                    }
                }
            }

            Kirigami.Heading {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Authors")
                textFormat: Text.PlainText
                visible: page.metaData.authors.length > 0
            }
            Repeater {
                model: page.metaData.authors
                delegate: personDelegate
            }

            Kirigami.Heading {
                height: visible ? implicitHeight : 0
                Layout.topMargin: Kirigami.Units.smallSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Credits")
                textFormat: Text.PlainText
                visible: repCredits.count > 0
            }
            Repeater {
                id: repCredits
                model: page.metaData.otherContributors
                delegate: personDelegate
            }

            Kirigami.Heading {
                height: visible ? implicitHeight : 0
                Layout.topMargin: Kirigami.Units.smallSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Translators")
                textFormat: Text.PlainText
                visible: repTranslators.count > 0
            }
            Repeater {
                id: repTranslators
                model: page.metaData.translators
                delegate: personDelegate
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter

                icon.name: "tools-report-bug"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Report a Bug…")

                visible: page.metaData.bugReportUrl.length > 0

                onClicked: Qt.openUrlExternally(page.metaData.bugReportUrl)
            }
        }
    }
}
