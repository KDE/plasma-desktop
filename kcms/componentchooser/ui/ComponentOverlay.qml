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

    function sanitize(input: string): string {
        // Based on QQuickStyledTextPrivate::parseEntity
        const table = {
            '>': '&gt;',
            '<': '&lt;',
            '&': '&amp;',
            "'": '&apos;',
            '"': '&quot;',
            '\u00a0': '&nbsp;',
        };
        return input.replace(/[<>&'"\u00a0]/g, c => table[c]);
    }

    function formatListItem(item: string): string {
        return "<li>" + sanitize(item) + "</li>";
    }

    function formatHtmlUnorderedStringList(list: list<string>): string {
        return "<ul>" + list.map(formatListItem).join("\n") + "</ul>";
    }

    onOpened: {
        focus = true;
    }

    onClosed: {
        componentChooser = null;
    }

    title: i18n("Details")

    modal: true
    QQC2.Overlay.modal: KcmPopupModal {}

    ColumnLayout {
        enabled: root.componentChooser !== null
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            text: i18n("This application does not advertise support for the following file types:")
            visible: root.unsupportedMimeTypes.length > 0
            level: 3
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
        }
        Kirigami.SelectableLabel {
            visible: root.unsupportedMimeTypes.length > 0
            text: root.formatHtmlUnorderedStringList(root.unsupportedMimeTypes)
            Layout.fillWidth: true
        }
        QQC2.Button {
            visible: root.unsupportedMimeTypes.length > 0
            text: i18nc("@action:button", "Force Open Anyway")
            Layout.topMargin: Kirigami.Units.largeSpacing
            onClicked: {
                root.close();
                root.componentChooser.saveAssociationUnsuportedMimeTypes();
            }
        }

        Kirigami.Separator {
            // extra double-spacing
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            visible: root.unsupportedMimeTypes.length > 0
                && root.mimeTypesNotAssociated.length > 0
        }

        Kirigami.Heading {
            text: i18n("The following file types are still associated with a different application:")
            visible: root.mimeTypesNotAssociated.length > 0
            level: 3
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
        }
        Kirigami.SelectableLabel {
            visible: root.mimeTypesNotAssociated.length > 0
            text: root.formatHtmlUnorderedStringList(
                root.mimeTypesNotAssociated.map(
                    ({first, second}) => i18nc(
                        "@label %1 is a MIME type and %2 is an application name",
                        "%1 associated with %2",
                        second,
                        first,
                    )
                )
            )
            Layout.fillWidth: true
        }
        QQC2.Button {
            visible: root.mimeTypesNotAssociated.length > 0
            icon.name: root.componentChooser?.applicationIcon() ?? ""
            text: i18nc(
                "@action:button %1 is an application name",
                "Re-assign-all to %1",
                root.componentChooser?.applicationName() ?? ""
            )
            Layout.topMargin: Kirigami.Units.largeSpacing
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
