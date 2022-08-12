import QtQuick 2.12
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.15
import Qt.labs.platform 1.1 as Platform
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrolsaddons 2.0 as KQCAddons
import org.kde.plasma.workspace.keyboardlayout 1.0

Kirigami.FormLayout {
    id: root

    property alias cfg_showFlag: showFlag.checked
    readonly property string layoutShortName: keyboardLayout.layoutsList.length ? keyboardLayout.layoutsList[keyboardLayout.layout].shortName
                                                                             : ""
    readonly property string displayName: keyboardLayout.layoutsList.length ? keyboardLayout.layoutsList[keyboardLayout.layout].displayName
                                                                             : ""
    KeyboardLayout { id: keyboardLayout }

    RadioButton {
        id: showLabel
        Kirigami.FormData.label: i18n("Display style:")
        checked: true
        text: root.displayName.length > 0 ? root.displayName: root.layoutShortName
    }

    RadioButton {
        id: showFlag
        contentItem: Item {
            implicitWidth: childrenRect.width + showFlag.indicator.width
            implicitHeight: childrenRect.height

            Image {
                id: flagImage
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                source: Platform.StandardPaths.locate(Platform.StandardPaths.GenericDataLocation,
                                                      "kf5/locale/countries/" + root.layoutShortName + "/flag.png")
                visible: status == Image.Ready
            }
            RowLayout {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: Kirigami.Units.smallSpacing
                width: visible ? implicitWidth : 0
                height: visible ? implicitHeight : 0
                visible: !flagImage.visible

                Kirigami.Icon {
                    implicitWidth: Kirigami.Units.iconSizes.smallMedium
                    implicitHeight: Kirigami.Units.iconSizes.smallMedium
                    source: "emblem-warning"
                }
                Label {
                    text: i18nc("@info:placeholder Make this translation as short as possible", "No flag available")
                }
            }
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    Button {
        Kirigami.FormData.label: i18n("Layouts:")
        text: i18n("Configure…")
        icon.name: "configure"
        onClicked: KQCAddons.KCMShell.openSystemSettings("kcm_keyboard", "--tab=layouts")
    }

    Component.onCompleted: {
        // hide Keyboard Shortcuts tab
        var appletConfiguration = app
        while (appletConfiguration.parent) {
            appletConfiguration = appletConfiguration.parent
        }
        if (appletConfiguration && typeof appletConfiguration.globalConfigModel !== "undefined") {
            appletConfiguration.globalConfigModel.removeCategoryAt(0)
        }
    }
}
