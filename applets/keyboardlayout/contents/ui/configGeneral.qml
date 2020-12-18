import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {

    CheckBox {
        checked: plasmoid.configuration.showFlag
        onCheckedChanged: plasmoid.configuration.showFlag = checked
        text: "Show flag"
    }
}
