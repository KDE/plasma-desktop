import QtQuick 2.6
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.3 as Kirigami

Column {

    property int tristate;
    signal changed();

    Controls.RadioButton {
        id: turn_on;
        text: i18n("Turn on");
        checked: tristate == 0;
        onClicked: {
            tristate = 0;
            changed();
        }
    }
    Controls.RadioButton {
        id: turn_off;
        text: i18n("Turn off");
        checked: tristate == 1;
        onClicked: {
            tristate = 1;
            changed();
        }
    }
    Controls.RadioButton {
        id: leave_unchanged;
        text: i18n("Leave unchanged");
        checked: tristate == 2;
        onClicked: {
            tristate = 2;
            changed();
        }
    }
}
