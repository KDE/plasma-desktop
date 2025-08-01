/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.landingpage.kcm

Column {
    id: root
    Accessible.role: Accessible.RadioButton
    Accessible.name: text
    Accessible.onPressAction: radioButton.toggle();
    Accessible.onToggleAction: radioButton.toggle();

    required property ButtonGroup group
    required property string packageId
    required property Component availablePackages

    property alias checked: radioButton.checked

    readonly property alias preview: previewImage
    readonly property string text: metaData.name
    readonly property int implicitButtonHeight: Kirigami.Units.gridUnit * 5
    readonly property int implicitButtonWidth: implicitButtonHeight * 1.6

    signal toggled()
    signal accepted(id: string)

    spacing: Kirigami.Units.smallSpacing

    Kirigami.ShadowedRectangle {
        id: delegate
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        implicitWidth: root.implicitButtonWidth + toolButton.implicitWidth
        implicitHeight: root.implicitButtonHeight
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.cornerRadius
        shadow.xOffset: 0
        shadow.yOffset: 2
        shadow.size: 10
        shadow.color: Qt.rgba(0, 0, 0, 0.3)

        Row {
            anchors.fill: parent

            RadioButton {
                id: radioButton
                ButtonGroup.group: root.group
                implicitWidth: root.implicitButtonWidth
                implicitHeight: root.implicitButtonHeight

                background: Item {}
                indicator: Item {}

                contentItem: Item {
                    Rectangle {
                        anchors.fill: parent
                        topLeftRadius: Kirigami.Units.cornerRadius
                        bottomLeftRadius: Kirigami.Units.cornerRadius
                        topRightRadius: 0
                        bottomRightRadius: 0
                        color: {
                            if (radioButton.checked) {
                                return Kirigami.Theme.highlightColor;
                            } else if (radioButton.hovered) {
                                return Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.5);
                            } else {
                                return Kirigami.Theme.backgroundColor;
                            }
                        }
                    }

                    Image {
                        id: previewImage
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.smallSpacing
                        asynchronous: true
                        layer.enabled: true
                        sourceSize: Qt.size(width * Screen.devicePixelRatio, height * Screen.devicePixelRatio)
                        source: metaData.preview
                    }
                }

                onToggled: root.toggled()
            }

            AbstractButton {
                id: toolButton
                implicitWidth: Kirigami.Units.iconSizes.small + leftPadding + rightPadding
                implicitHeight: root.implicitButtonHeight
                leftPadding: Kirigami.Units.smallSpacing
                rightPadding: Kirigami.Units.smallSpacing

                text: i18nc("@action:button", "Change global theme")
                display: AbstractButton.IconOnly

                Accessible.role: Accessible.ButtonMenu

                contentItem: Kirigami.Icon {
                    source: "arrow-down"
                }

                background: Rectangle {
                    color: {
                        if (toolButton.pressed || popup.visible) {
                            return Kirigami.Theme.highlightColor;
                        } else if (toolButton.hovered || toolButton.visualFocus) {
                            return Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.5);
                        } else {
                            return Kirigami.Theme.backgroundColor;
                        }
                    }
                    topLeftRadius: 0
                    bottomLeftRadius: 0
                    topRightRadius: Kirigami.Units.cornerRadius
                    bottomRightRadius: Kirigami.Units.cornerRadius
                }

                onClicked: {
                    if (popup.visible) {
                        popup.close();
                    } else {
                        popup.open();
                    }
                }
            }
        }
    }

    Label {
        id: label
        width: delegate.implicitWidth
        text: root.text
        textFormat: Text.PlainText
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.paintedWidth
            height: 1
            color: Kirigami.Theme.highlightColor
            opacity: radioButton.visualFocus ? 1 : 0
        }
    }

    Popup {
        id: popup
        y: delegate.height
        x: delegate.width - width
        implicitHeight: Math.min(contentItem.implicitHeight + topPadding + bottomPadding, Kirigami.Units.gridUnit * 40)
        focus: true
        popupType: Popup.Native
        padding: 1
        contentItem: ScrollView {
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            contentWidth: listView.implicitWidth
            contentHeight: listView.implicitHeight
            focus: true

            background: Rectangle {
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                color: Kirigami.Theme.backgroundColor
            }

            ListView {
                id: listView
                Accessible.role: Accessible.List // TODO: remove once Qt sets this automatically
                Accessible.name: i18nc("@label accessible", "Global theme")
                focus: true
                implicitWidth: radioButton.width
                implicitHeight: contentHeight
                delegate: ItemDelegate {
                    id: delegate

                    readonly property string pluginId: model.packageId
                    readonly property string previewUrl: model.preview

                    text: model.name

                    width: ListView.view.width
                    contentItem: Column {
                        spacing: Kirigami.Units.smallSpacing

                        Image {
                            width: parent.width
                            height: width / 1.6
                            source: delegate.previewUrl
                        }

                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            text: delegate.text
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                        }
                    }

                    onClicked: {
                        root.accepted(delegate.pluginId);
                        popup.close();
                    }
                }
            }
        }

        onAboutToShow: {
            listView.model = root.availablePackages.createObject(listView);
            listView.currentIndex = listView.model.indexOf(root.packageId);
        }

        onClosed: {
            if (listView.model) {
                listView.model.destroy();
                listView.model = null;
            }
        }
    }

    LookAndFeelMetaData {
        id: metaData
        packageId: root.packageId
    }
}
