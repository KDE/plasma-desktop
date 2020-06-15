import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.1 as KCM

KCM.SimpleKCM {
    id: root

    property var dataModel;
    signal changed();

    Kirigami.FormLayout {
        id: formLayout

        Controls.ComboBox {
            id: keyboardModelComboBox
            Kirigami.FormData.label: i18n("Keyboard Model:")
            model: dataModel.keyboardModelModel
            textRole: "description"
            implicitWidth: textarea.width

            Binding on currentIndex {
                value: dataModel.currentIndex
            }

            MouseArea {
                anchors.fill: parent
                onWheel: {
                    // disable wheel events
                }
                onPressed: {
                    // propogate to ComboBox
                    mouse.accepted = false;
                }
                onReleased: {
                    // propogate to ComboBox
                    mouse.accepted = false;
                }
            }
            onActivated: {
                dataModel.currentIndex = currentIndex;
                root.changed();
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }
        
        Controls.TextField {
            Kirigami.FormData.label: i18n("Test area:")
            id: textarea
            wrapMode: TextEdit.Wrap
        }
        
        Controls.ComboBox {
            Kirigami.FormData.label: i18n("On start:")
            model: [
                i18n("Numlock | On"),
                i18n("Numlock | Off"),
                i18n("Numlock | Leave unchanged")
            ]
            
            Binding on currentIndex {
                value: dataModel.numlockOnStartup
            }
            
            onAccepted: {
                dataModel.numlockOnStartup = currentIndex;
                root.changed();
            }
        }
        
        Controls.ComboBox {
            model: [
                i18n("Keyboard Repeat | On"),
                i18n("Keyboard Repeat | Off"),
                i18n("Keyboard Repear | Leave unchanged")
            ]
            
            Binding on currentIndex {
                value: dataModel.keyboardRepeat
            }
            
            onAccepted: {
                dataModel.keyboardRepeat = currentIndex;
                root.changed();
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            id: delay
            Kirigami.FormData.label: i18n("Delay:")

            Controls.Slider {
                id: delay_slider

                enabled: dataModel.keyboardRepeat === 0

                Layout.fillWidth: true;
                width: 300
                from: 100
                value: dataModel.repeatDelay
                to: 5000

                onMoved: {
                    dataModel.repeatDelay = value;
                    root.changed();
                }
            }

            Controls.TextField {
                enabled: dataModel.keyboardRepeat === 0
                Layout.fillWidth: false;
                Layout.preferredWidth: 150;
                id: delay_number_field;
                readOnly: true;
                text: Math.floor(delay_slider.value) + " ms";
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Rate:")

            Controls.Slider {
                id: rate_slider

                enabled: dataModel.keyboardRepeat === 0

                Layout.fillWidth: true;
                width: 300
                from: 0.2
                value: dataModel.repeatRate
                to: 50

                onMoved: {
                    dataModel.repeatRate = value;
                    root.changed();
                }
            }

            Controls.TextField {
                enabled: dataModel.keyboardRepeat === 0
                Layout.fillWidth: false;
                Layout.preferredWidth: 150;
                id: rate_number_field;
                readOnly: true;
                text: parseFloat(rate_slider.value).toFixed(2) + " repeats/s";
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }
    }
}
