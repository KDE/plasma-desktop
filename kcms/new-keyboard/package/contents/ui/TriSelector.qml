import QtQuick 2.1
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.3 as Kirigami

Column {
    Kirigami.FormData.label: i18n("Numlock on Plasma Startup:")

    Controls.RadioButton {
        id: turn_on;
        text: i18n("Turn on");
    }
    Controls.RadioButton {
        id: turn_off;
        text: i18n("Turn off");
    }
    Controls.RadioButton {
        id: leave_unchanged;
        text: i18n("Leave unchanged");
    }
}
