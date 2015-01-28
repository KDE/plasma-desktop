/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.private.trash 1.0 as TrashPrivate

MouseArea {
    id:root

    Layout.minimumWidth: formFactor == PlasmaCore.Types.Horizontal ? height: 1
    Layout.minimumHeight: formFactor == PlasmaCore.Types.Vertical ? width : 1
    property int formFactor: plasmoid.formFactor
    property bool constrained: formFactor==PlasmaCore.Types.Vertical||formFactor==PlasmaCore.Types.Horizontal
    hoverEnabled: true
    onClicked: Qt.openUrlExternally("trash:/");
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Plasmoid.backgroundHints: PlasmaCore.Types.TranslucentBackground
    TrashPrivate.DirModel {
        id: dirModel
        url: "trash:/"
        onCountChanged: {
            plasmoid.action("empty").enabled = count > 0;
        }
    }

    function action_open() {
        Qt.openUrlExternally("trash:/");
    }

    function action_empty() {
        queryDialog.open();
    }

    Component.onCompleted: {
        plasmoid.setAction("open", i18nc("a verb", "Open"),"document-open");
        plasmoid.setAction("empty",i18nc("a verb", "Empty"),"trash-empty");
        plasmoid.popupIcon = "user-trash";
        plasmoid.action("empty").enabled = count > 0;
    }

    PlasmaCore.IconItem {
        id:icon
        source: (dirModel.count > 0) ? "user-trash-full": "user-trash"
        anchors{
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: constrained ? parent.bottom: text.top
        }
        active: root.containsMouse
    }
    Components.Label {
        id: text
        text: (dirModel.count==0) ? i18n("Trash\nEmpty"): i18np("Trash\nOne item", "Trash\n %1 items", dirModel.count);
        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
        }
        horizontalAlignment: Text.AlignHCenter
        opacity: constrained ? 0: 1
    }
    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        mainText: i18n("Trash")
        subText: (dirModel.count==0) ? i18n("Trash \n Empty"): i18np("Trash\nOne item", "Trash\n %1 items", dirModel.count );
        icon: (dirModel.count > 0) ? "user-trash-full": "user-trash"
    }


    Components.QueryDialog {
        id: queryDialog
        titleIcon: "user-trash"
        titleText: i18n("Empty Trash")
        message: i18n("Do you really want to empty the trash ? All the items will be deleted.")
        acceptButtonText: i18n("Empty Trash")
        rejectButtonText: i18n("Cancel")
        onAccepted: dirModel.emptyTrash();
        onRejected: queryDialog.close();
        visualParent: root
    }
}
