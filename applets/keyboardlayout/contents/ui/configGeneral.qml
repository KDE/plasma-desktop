import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    property alias cfg_showFlag: showFlagCheckBox.checked

    CheckBox {
        id: showFlagCheckBox
        text: i18n("Show flag")
    }
}
