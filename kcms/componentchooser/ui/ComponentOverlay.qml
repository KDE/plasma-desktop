/*
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

Kirigami.OverlaySheet {
    id: root

    property QtObject componentChooser

    // type: list<string>
    property var unsupportedMimeTypes: componentChooser?.unsupportedMimeTypes ?? []
    // type: list<{ first, second }>
    property var mimeTypesNotAssociated: componentChooser?.mimeTypesNotAssociated ?? []

    title: i18n("Details")

    modal: true
    QQC2.Overlay.modal: KcmPopupModal {}

    ColumnLayout {
        enabled: root.componentChooser !== null
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18n("This application does not advertise support for the following file types:")
            visible: root.unsupportedMimeTypes.length > 0
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        ListView {
            visible: root.unsupportedMimeTypes.length > 0
            implicitHeight: contentHeight
            model: root.unsupportedMimeTypes
            delegate: QQC2.Label {
                text: modelData
                width: ListView.view.width
            }
            Layout.fillWidth: true
        }
        QQC2.Button {
            visible: root.unsupportedMimeTypes.length > 0
            text: i18nc("@action:button", "Force Open Anyway")
            onClicked: {
                root.close();
                root.componentChooser.saveAssociationUnsuportedMimeTypes();
            }
        }

        QQC2.Label {
            text: i18n("The following file types are still associated with a different application:")
            visible: root.mimeTypesNotAssociated.length > 0
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        ListView {
            visible: root.mimeTypesNotAssociated.length > 0
            implicitHeight: contentHeight
            model: root.mimeTypesNotAssociated
            delegate: QQC2.Label {
                text: i18nc(
                    "@label %1 is a MIME type and %2 is an application name",
                    "%1 associated with %2",
                    modelData.second,
                    modelData.first,
                )
                width: ListView.view.width
            }
            Layout.fillWidth: true
        }
        QQC2.Button {
            visible: root.mimeTypesNotAssociated.length > 0

            text: i18nc(
                "@action:button %1 is an application name",
                "Re-assign-all to %1",
                root.componentChooser?.applicationName() ?? ""
            )
            onClicked: {
                root.close();
                root.componentChooser.saveMimeTypesNotAssociated();
            }
        }

        QQC2.Button {
            text: i18n("Change file type association manually")
            visible: root.componentChooser !== null
            onClicked: {
                root.close();
                KCM.KCMLauncher.openSystemSettings("kcm_filetypes");
            }
        }
    }
}
