/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls

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
            required property var model // for display, which shadows a delegate property
            required checked
            width: ListView.view.width
            text: model.display

            onToggled: model.checked = checked;
        }
    }

    Component.onCompleted: {
        previewPluginsModel.checkedPlugins = dialog.previewPlugins;
        open();
    }
}
