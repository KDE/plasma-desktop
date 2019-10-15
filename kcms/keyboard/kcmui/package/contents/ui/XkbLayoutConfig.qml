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

    ListView {
        id: configList;
        model: configModel

        anchors.fill: parent

        clip: true

        delegate: RowLayout {
            Controls.Label {
                text: model.description
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
