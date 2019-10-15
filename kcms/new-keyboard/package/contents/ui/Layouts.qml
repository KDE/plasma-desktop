import QtQuick 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.1 as KCM

KCM.SimpleKCM {
    id: root

    anchors.fill: parent

    Kirigami.FormLayout {
        id: formLayout

        Column {
            Kirigami.FormData.label: i18n("Layout Indicator")

            Controls.CheckBox {
                id: layout_indicator_show;
                text: i18n("Show layout indicator");
            }
            Controls.CheckBox {
                id: single_layout_show;
                text: i18n("Show for single layout");
            }
        }

        Column {
            Kirigami.FormData.label: i18n("Layout Indicator Icon")

            Controls.RadioButton {
                id: show_flag;
                text: i18n("Show flag");
            }
            Controls.RadioButton {
                id: show_label;
                text: i18n("Show label");
            }
            Controls.RadioButton {
                id: show_label_on_flag;
                text: i18n("Show label on flag");
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Switching Policy")

            model: [i18n("Global"), i18n("Desktop"), i18n("Application"), i18n("Window")]
        }

        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Shortcuts for Switching Layout")
        }

        Controls.Button {
            Kirigami.FormData.label: i18n("Main shortcuts")
            implicitWidth: 200
        }
        Controls.Button {
            Kirigami.FormData.label: i18n("3rd level shortcut")
            implicitWidth: 200
        }
        Controls.Button {
            Kirigami.FormData.label: i18n("Alternative shortcut")
            implicitWidth: 200
        }

        ListView {
            id: layout_list
            Kirigami.FormData.isSection: true

            contentWidth: headerItem.width

            header: Row {
                spacing: 1
                function itemAt(index) { return repeater.itemAt(index) }
                Repeater {
                    id: repeater
                    model: ["Map", "Layout", "Variant", "Label", "Shortcut"]
                    Controls.Label {
                        text: modelData
                        font.bold: true
                        padding: 5
                        background: Rectangle { color: "white" }
                    }
                }
            }
        }


    }
}
