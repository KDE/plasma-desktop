/*
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.3 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons


Kirigami.OverlaySheet {

    id: root;
    property var componenChooser;

    property var unsupportedMimeTypes: componenChooser ? componenChooser.unsupportedMimeTypes: []
    property var mimeTypesNotAssociated: componenChooser ? componenChooser.mimeTypesNotAssociated: []

    title: i18n("Details")

    ColumnLayout {
        enabled: componenChooser != undefined

        QQC2.Label {
            text: i18n("This application does not advertise support for the following file types:")
            visible: unsupportedMimeTypesListView.visible
        }
        ListView {
            id: unsupportedMimeTypesListView
            visible: unsupportedMimeTypes.length > 0
            implicitHeight: unsupportedMimeTypes.length * Kirigami.Units.gridUnit
            model: unsupportedMimeTypes
            delegate: Row {
                QQC2.Label { text: modelData }
            }
        }
        QQC2.Button {
            visible: unsupportedMimeTypesListView.visible
            text: i18nc("@action:button", "Force Open Anyway")
            onClicked: {
                root.close();
                componenChooser.saveAssociationUnsuportedMimeTypes();
            }
        }

        QQC2.Label {
            text: i18n("The following file types are still associated with a different application:")
            visible: notAssociatedlistView.visible
        }
        ListView {
            id: notAssociatedlistView
            visible: mimeTypesNotAssociated.length > 0
            implicitHeight: mimeTypesNotAssociated.length * Kirigami.Units.gridUnit;
            model: mimeTypesNotAssociated
            delegate: Row {
                QQC2.Label { text: i18nc("@label %1 is a MIME type and %2 is an application name", "%1 associated with %2", modelData.second, modelData.first)}
            }
        }
        QQC2.Button {
            visible: componenChooser ? notAssociatedlistView.visible : false

            text: i18nc("@action:button %1 is an application name", "Re-assign-all to %1", componenChooser ? componenChooser.applicationName() : "")
            onClicked: {
                root.close();
                componenChooser.saveMimeTypesNotAssociated();
            }
        }

        QQC2.Button {
            text: i18n("Change file type association manually")
            visible: componenChooser ? true : false
            onClicked: {
                root.close();

                KQuickControlsAddons.KCMShell.openSystemSettings("kcm_filetypes", componenChooser.storageId());
            }
        }
    }
}
