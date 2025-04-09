/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.private.desktopcontainment.folder as Folder
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: dialog

    required property var previewPlugins

    title: i18n("Preview Plugins")

    preferredWidth: Kirigami.Units.gridUnit * 15
    implicitHeight: Math.round(parent.height * 0.8)
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    onAccepted: {
        configIcons.cfg_previewPlugins = previewPluginsModel.checkedPlugins;
        dialog.close();
    }
    onRejected: {
        dialog.close();
        destroy();
    }
    onClosed: destroy()

    ListView {
        model: Folder.PreviewPluginsModel {
            id: previewPluginsModel
        }

        delegate: CheckDelegate {
            width: ListView.view.width
            text: model.display

            checked: model.checked
            onToggled: model.checked = checked;
        }
    }

    Component.onCompleted: {
        previewPluginsModel.checkedPlugins = dialog.previewPlugins;
        open();
    }
}
