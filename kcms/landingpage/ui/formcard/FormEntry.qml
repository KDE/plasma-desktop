import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FC

Item {
    id: root

    property string title: contentItem?.Kirigami.FormData.label
    property string subtitle
    property alias separatorVisible: separator.visible

    property alias contentItem: layout.contentItem
    property alias background: impl.background
    property alias footer: layout.footer

    implicitWidth: impl.implicitWidth
    implicitHeight: impl.implicitHeight

    Layout.fillWidth: true

    signal clicked

    T.ItemDelegate {
        id: impl
        anchors.fill: parent
        implicitWidth: layout.implicitWidth + padding * 2
        implicitHeight: layout.implicitHeight + padding * 2
        padding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

        onClicked: {
            const buddy = root.contentItem?.Kirigami.FormData.buddyFor;
            buddy.forceActiveFocus(Qt.ShortcutFocusReason);
            if (buddy instanceof T.AbstractButton) {
                buddy.animateClick();
            } else if (buddy instanceof T.ComboBox) {
                buddy.popup.open();
            }
        }

        contentItem: Kirigami.HeaderFooterLayout {
            id: layout
            spacing: Kirigami.Units.smallSpacing

            header: QQC.Label {
                visible: text.length > 0
                Kirigami.MnemonicData.enabled: {
                    const buddy = root.contentItem?.Kirigami.FormData.buddyFor;
                    if (buddy && buddy.enabled && buddy.visible && buddy.activeFocusOnTab) {
                        // Only set mnemonic if the buddy doesn't already have one.
                        const buddyMnemonic = buddy.Kirigami.MnemonicData;
                        return !buddyMnemonic.label || !buddyMnemonic.enabled;
                    } else {
                        return false;
                    }
                }
                Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.FormLabel
                Kirigami.MnemonicData.label: root.title
                text: Kirigami.MnemonicData.richTextLabel
                Accessible.name: Kirigami.MnemonicData.plainTextLabel
                Shortcut {
                    sequence: layout.header.Kirigami.MnemonicData.sequence
                    onActivated: {
                        const buddy = root.contentItem?.Kirigami.FormData.buddyFor;

                        const buttonBuddy = buddy as T.AbstractButton;
                        // animateClick is only in Qt 6.8,
                        // it also takes into account focus policy.
                        if (buttonBuddy && buttonBuddy.animateClick) {
                            buttonBuddy.animateClick();
                        } else {
                            buddy.forceActiveFocus(Qt.ShortcutFocusReason);
                        }
                    }
                }
            }
            footer: QQC.Label {
                visible: text.length > 0
                text: root.subtitle
                opacity: 0.6
            }
        }

        background: Rectangle {
            color: Kirigami.Theme.textColor
            opacity: impl.hovered && root.contentItem?.Kirigami.FormData.buddyFor instanceof T.AbstractButton? 0.05 : 0
            readonly property bool first: root.parent.children[0] === root
            readonly property bool last: root.parent.children[root.parent.children.length - 1] === root
            topLeftRadius: first ? Kirigami.Units.cornerRadius : 0
            topRightRadius: topLeftRadius
            bottomLeftRadius: last ? Kirigami.Units.cornerRadius : 0
            bottomRightRadius: bottomLeftRadius
            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    // TODO: this should be a property in the template
    Kirigami.Separator {
        id: separator
        opacity: 0.5
        visible: !(root.contentItem?.Kirigami.FormData.buddyFor instanceof QQC.RadioButton) || !(root.parent.children[root.parent.children.indexOf(root) + 1].contentItem?.Kirigami.FormData.buddyFor instanceof QQC.RadioButton)
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing
        }
    }
}
