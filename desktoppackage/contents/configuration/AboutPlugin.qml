/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
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
                Layout.fillWidth: true
                text: modelData.name
                textFormat: Text.PlainText
            }
            Row {
                // Group action buttons together
                spacing: 0
                QQC2.Button {
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
                QQC2.Button {
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
            id: column

            readonly property int headingTopSpacing: Kirigami.Units.smallSpacing
            readonly property int dataLeftSpacing: Kirigami.Units.smallSpacing

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
                    Layout.alignment: Qt.AlignBottom
                    text: page.metaData.name + " " + page.metaData.version
                    textFormat: Text.PlainText
                }

                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 15
                    level: 3
                    type: Kirigami.Heading.Type.Secondary
                    wrapMode: Text.WordWrap
                    text: page.metaData.description
                    textFormat: Text.PlainText
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }

            Kirigami.Heading {
                Layout.topMargin: column.headingTopSpacing
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@title:group", "Website")
                textFormat: Text.PlainText
            }
            Kirigami.UrlButton {
                Layout.leftMargin: column.dataLeftSpacing
                url: page.metaData.website
                visible: url.length > 0
            }

            Kirigami.Heading {
                Layout.topMargin: column.headingTopSpacing
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@title:group", "Get Help")
                textFormat: Text.PlainText
            }
            Kirigami.UrlButton {
                Layout.leftMargin: column.dataLeftSpacing
                textFormat: Text.PlainText

                url: page.metaData.bugReportUrl
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Report an issue")

                visible: page.metaData.bugReportUrl.length > 0
            }

            Kirigami.Heading {
                Layout.topMargin: column.headingTopSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Copyright")
                textFormat: Text.PlainText
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.leftMargin: column.dataLeftSpacing

                QQC2.Label {
                    text: page.metaData.copyrightText
                    textFormat: Text.PlainText
                    visible: text.length > 0
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: i18nd("plasma_shell_org.kde.plasma.desktop %1 is the short SPDX text for the license", "License: %1", page.metaData.license)
                        textFormat: Text.PlainText
                    }

                    QQC2.Button {
                        icon.name: "view-readermode"
                        text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button", "Read License")
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
                Layout.topMargin: column.headingTopSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Authors")
                textFormat: Text.PlainText
                visible: page.metaData.authors.length > 0
            }
            Repeater {
                Layout.leftMargin: column.dataLeftSpacing
                model: page.metaData.authors
                delegate: personDelegate
            }

            Kirigami.Heading {
                height: visible ? implicitHeight : 0
                Layout.topMargin: column.headingTopSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Credits")
                textFormat: Text.PlainText
                visible: repCredits.count > 0
            }
            Repeater {
                id: repCredits
                Layout.leftMargin: column.dataLeftSpacing
                model: page.metaData.otherContributors
                delegate: personDelegate
            }

            Kirigami.Heading {
                height: visible ? implicitHeight : 0
                Layout.topMargin: column.headingTopSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Translators")
                textFormat: Text.PlainText
                visible: repTranslators.count > 0
            }
            Repeater {
                id: repTranslators
                Layout.leftMargin: column.dataLeftSpacing
                model: page.metaData.translators
                delegate: personDelegate
            }
        }
    }
}
