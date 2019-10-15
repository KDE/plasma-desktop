import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.2

Item {
    id: rootItem;

    signal changed();

    Controls.TabBar {
        id: tabbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        Controls.TabButton {
            text: i18n("Hardware")
        }
        Controls.TabButton {
            text: i18n("Layouts")
        }
        Controls.TabButton {
            text: i18n("Advanced")
        }
    }

    StackLayout {
        anchors.top: tabbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: tabbar.currentIndex

        Hardware {
            dataModel: kcm.hardwareModel
            onChanged: rootItem.changed();
        }

        Layouts {
            dataModel: kcm.layoutModel
            onChanged: rootItem.changed();
        }

        Advanced {
            dataModel: kcm.advancedModel
            onChanged: rootItem.changed();
        }
    }
}
