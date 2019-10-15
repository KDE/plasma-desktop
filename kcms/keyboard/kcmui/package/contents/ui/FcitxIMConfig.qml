import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.4 as Kirigami

Controls.Dialog {
    header: Kirigami.Heading { text: i18n("IM Config") }
    id: root
    property var configModel;

    function openForModel(model) {
        configModel = model;
        open();
    }

    ColumnLayout {
        anchors.fill: parent

        ColumnLayout {
            id: header
            visible: false
            Layout.fillHeight: true

            Connections {
                target: root
                onOpened: {
                    if (configModel.isRealIM) {
                        header.visible = true;

                        latinCheckbox.checked = configModel.isLatinSwitchingEnabled
                        latinLayoutList.model = configModel.latinModeLayoutList
                        latinLayoutList.currentIndex = configModel.selectedLatinLayoutIndex
                    }
                    else {
                        header.visible = false;
                    }
                }
            }

            Controls.CheckBox {
                id: latinCheckbox
                text: i18n("Enable Switching to Latin")
                onCheckedChanged: configModel.isLatinSwitchingEnabled = checked;
            }

            Controls.ComboBox {
                enabled: latinCheckbox.checked
                id: latinLayoutList
                textRole: "description"
                Layout.minimumWidth: 500
                onActivated: configModel.selectedLatinLayoutIndex = currentIndex
            }
        }

        ListView {
            id: configList;
            model: configModel

            width: parent.width
            Layout.fillHeight: true

            clip: true

            Component {
                id: sectionHeading
                Rectangle {
                    width: parent.width
                    height: childrenRect.height
                    color: "transparent"

                    Text {
                        text: section
                        font.bold: true
                        font.pixelSize: 20
                    }
                }
            }

            section.property: "group"
            section.delegate: sectionHeading

            delegate: RowLayout {
                id: itemDelegate
                property var model_: model;
                height: 40
                spacing: 10

                Controls.Label {
                    Layout.minimumWidth: 300
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignRight
                    text: model.description;
                }

                Component.onCompleted: {
                    switch (model.type) {
                    case "EnumType":
                        Qt.createQmlObject(
                            "import QtQuick 2.9;
                             import QtQuick.Controls 2.3 as Controls;
                             Controls.ComboBox {
                                model: model_.data;
                                currentIndex: model_.data.indexOf(model_.current_value);
                                onActivated: model_.current_value = currentText;
                             }", itemDelegate, "dynamicSnippet1");
                        break;
                    case "BooleanType":
                        Qt.createQmlObject(
                            "import QtQuick 2.9;
                             import QtQuick.Controls 2.3 as Controls;
                             Controls.CheckBox {
                                checked: model_.current_value
                                onCheckedChanged: model_.current_value = checked;
                             }", itemDelegate, "dynamicSnippet2");
                        break;
                    case "HotkeyType":
                        Qt.createQmlObject(
                            "import QtQuick 2.9;
                             import org.kde.kquickcontrols 2.0 as KQuickControls;
                             KQuickControls.KeySequenceItem {
                                keySequence: model_.current_value
                                onKeySequenceChanged: model_.current_value = keySequence;
                             }", itemDelegate, "dynamicSnippet3");
                        break;
                    case "IntegerType":
                        Qt.createQmlObject(
                           "import QtQuick 2.9;
                            import QtQuick.Controls 2.3 as Controls;
                            Controls.TextField {
                                text: model_.current_value;
                                onTextChanged: model_.current_value = text;
                            }", itemDelegate, "dynamicSnippet4");
                       break;
                    case "StringType":
                    case "CharType":
                        Qt.createQmlObject(
                           "import QtQuick 2.9;
                            import QtQuick.Controls 2.3 as Controls;
                            Controls.TextField {
                                text: model_.current_value;
                                onTextChanged: model_.current_value = text;
                            }", itemDelegate, "dynamicSnippet4");
                       break;
                    }
                }
            }
        }
    }

    footer: RowLayout {
        Controls.Button {
            text: i18n("Save");
            onClicked: {
                configList.model.save();
                close();
            }
        }
        Controls.Button {
            text: i18n("Discard");
            onClicked: {
                close();
            }
        }
    }
}
