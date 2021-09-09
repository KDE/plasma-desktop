import QtQuick 2.12
import QtQuick.Controls 2.15
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrolsaddons 2.0

Kirigami.FormLayout {
    property alias cfg_showFlag: showFlag.checked

    RadioButton {
            text: i18n("Show label")
            checked: true
    }
    RadioButton {
            text: i18n("Show flag")
            id: showFlag
    }
    
    Button {
       text: i18n("Layout Settings")
       onClicked: {
         KCMShell.openSystemSettings("kcm_keyboard", "--tab=layouts");
       }
    }
}
