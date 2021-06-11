import QtQuick 2.12
import QtQuick.Controls 2.15
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrolsaddons 2.0

Kirigami.FormLayout {
    property alias cfg_showFlag: showFlagCheckBox.checked

    CheckBox {
        id: showFlagCheckBox
        text: i18n("Show flag")
    }
    
    Button {
       text: i18n("Layout Settings")
       onClicked: {
         KCMShell.openSystemSettings("kcm_keyboard", "--tab=layouts");
       }
    }
}
