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
    Accessible.onPressAction: button.toggle();
    Accessible.onToggleAction: button.toggle();

    required property ButtonGroup group
    required property string packageId
    required property Component availablePackages

    property alias checked: button.checked
    property alias popupEnabled: indicator.visible
    property alias preview: button.contentItem
    property string text: metaData.name

    readonly property alias previewImage: previewImage
    readonly property int implicitButtonHeight: Kirigami.Units.gridUnit * 5
    readonly property int implicitButtonWidth: implicitButtonHeight * 1.6
    readonly property alias hovered: button.hovered

    signal toggled()
    signal accepted(id: string)

    spacing: Kirigami.Units.smallSpacing

    AbstractButton {
        id: button

        implicitWidth: root.implicitButtonWidth + implicitIndicatorWidth
        implicitHeight: root.implicitButtonHeight

        ButtonGroup.group: root.group
        checkable: true

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        padding: Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.smallSpacing + implicitIndicatorWidth

        onToggled: root.toggled()

        contentItem: Image {
            id: previewImage
            sourceSize: Qt.size(width * Screen.devicePixelRatio, height * Screen.devicePixelRatio)
            source: metaData.preview
            layer.enabled: true
        }

        background: Kirigami.ShadowedRectangle {
            color: {
                if (button.checked) {
                    return Kirigami.Theme.highlightColor;
                } else if (button.hovered) {
                    return Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.5);
                } else {
                    return Kirigami.Theme.backgroundColor
                }
            }

            radius: Kirigami.Units.cornerRadius

            shadow {
                xOffset: 0
                yOffset: 2
                size: 10
                color: Qt.rgba(0, 0, 0, 0.3)
            }
        }

        indicator: AbstractButton {
            id: indicator

            anchors.right: parent.right

            implicitWidth: visible ? Kirigami.Units.iconSizes.small + leftPadding + rightPadding : 0
            implicitHeight: root.implicitButtonHeight

            padding: Kirigami.Units.smallSpacing

            visible: root.popupEnabled

            text: i18nc("@action:button", "Change global theme")
            display: AbstractButton.IconOnly

            Accessible.role: Accessible.ButtonMenu

            contentItem: Kirigami.Icon {
                source: "arrow-down"
            }

            background: Rectangle {
                color: {
                    if (indicator.pressed || popup.visible) {
                        return Kirigami.Theme.highlightColor;
                    } else if (indicator.hovered || indicator.visualFocus) {
                        return Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.5);
                    } else {
                        return Kirigami.Theme.backgroundColor;
                    }
                }

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

    Label {
        id: label
        width: button.implicitWidth
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
            opacity: button.visualFocus ? 1 : 0
        }
    }

    Popup {
        id: popup
        y: button.height
        implicitHeight: Math.min(contentItem.implicitHeight + topPadding + bottomPadding, Kirigami.Units.gridUnit * 40, root.Window.height - Kirigami.Units.gridUnit * 10)
        focus: true
        popupType: Popup.Item
        padding: 1
        clip: false

        contentItem: ScrollView {
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            contentWidth: listView.implicitWidth
            contentHeight: listView.implicitHeight
            focus: true
            clip: true

            bottomPadding: leftPadding > 0 ? leftPadding : Kirigami.Units.mediumSpacing

            ListView {
                id: listView
                Accessible.role: Accessible.List // TODO: remove once Qt sets this automatically
                Accessible.name: i18nc("@label accessible", "Global theme")
                focus: true
                implicitWidth: button.width
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

        background: Kirigami.ShadowedRectangle {
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View

            color: Kirigami.Theme.backgroundColor

            radius: Kirigami.Units.cornerRadius

            border.width: 1
            border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)

            shadow {
                size: 10
                yOffset: 2
                color: Qt.rgba(0, 0, 0, 0.2)
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
