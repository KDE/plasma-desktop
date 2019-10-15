import QtQuick 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.1 as KCM
import org.kde.kcm.keyboard 1.0

KCM.SimpleKCM {
    id: root

    anchors.fill: parent

    Kirigami.FormLayout {
        id: formLayout

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Keyboard Model:")
            model: KeyboardModelModel {}
            textRole: "description"
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        TriSelector {
            Kirigami.FormData.label: i18n("Numlock on Plasma Startup:")
        }

        TriSelector {
            Kirigami.FormData.label: i18n("Keyboard Repeat:")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            id: delay
            Kirigami.FormData.label: i18n("Delay:")

            Controls.Slider {
                id: delay_slider

                Layout.fillWidth: true;
                anchors.verticalCenter: parent.verticalCenter
                width: 300
                from: 100
                value: 10
                to: 5000
            }

            Controls.TextField {
                Layout.fillWidth: false;
                Layout.preferredWidth: 150;
                id: delay_number_field;
                text: Math.floor(delay_slider.value) + " ms"
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Rate:")

            Controls.Slider {
                id: rate_slider

                Layout.fillWidth: true;
                anchors.verticalCenter: parent.verticalCenter
                width: 300
                from: 0.2
                value: 10
                to: 50
            }

            Controls.TextField {
                Layout.fillWidth: false;
                Layout.preferredWidth: 150;
                id: rate_number_field;
                text: parseFloat(rate_slider.value).toFixed(2) + " repeats/s"
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        Controls.TextArea {
            Kirigami.FormData.label: i18n("Test area:")
            id: textarea
            implicitWidth: Math.max(parent.width / 3, delay.width)
            implicitHeight: 200
            wrapMode: TextEdit.Wrap
        }

    }
}
