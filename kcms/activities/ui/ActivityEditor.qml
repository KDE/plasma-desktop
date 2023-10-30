/*
 *  SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls
import org.kde.iconthemes 1.0 as KIconThemes

import org.kde.kcmutils as KCM
import org.kde.kcms.activities

KCM.SimpleKCM {

    property alias activityId: settings.activityId

    title: activityId ? i18nc("@title:window", "Activity Settings for %1", settings.name)
                      : i18nc("@title:window", "Create a New Activity")

    actions: Kirigami.Action {
        text: i18nc("@action:button as in, 'save changes'", "Save")
        icon.name: "document-save"
        enabled: settings.name.length > 0 && settings.isSaveNeeded
        onTriggered: saveContents();
    }

    Keys.onReturnPressed: saveContents();
    Keys.onEscapePressed: kcm.pop();

    function saveContents() {
        settings.save();
        kcm.pop();
    }

    ActivityConfig {
        id: settings
    }

    Kirigami.FormLayout {
        anchors.fill: parent

        Item {
            height: Kirigami.Units.smallSpacing
        }

        QQC2.Button {
            implicitHeight:  Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
            implicitWidth: height
            icon.height: Kirigami.Units.iconSizes.medium
            icon.width: Kirigami.Units.iconSizes.medium
            icon.name: settings.iconName
            Kirigami.FormData.label: i18nc("@label:chooser", "Icon:")
            onClicked: iconDialog.open();

            KIconThemes.IconDialog {
                id: iconDialog
                onIconNameChanged: iconName => settings.iconName = iconName
            }

        }

        QQC2.TextField {
            Kirigami.FormData.label: i18nc("@label:textbox", "Name:")
            text: settings.name
            onTextEdited: settings.name = text
        }

        QQC2.TextField {
            Kirigami.FormData.label: i18nc("@label:textbox", "Description:")
            text: settings.description
            onTextEdited: settings.description = text
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18nc("@option:check", "While on this activity, allow:")
            text: i18n("Tracking file and app use")
            checked: !settings.isPrivate
            onToggled: settings.isPrivate = !checked
        }

        QQC2.CheckBox {
            text: i18nc("@option:check", "Automatically turning off the screen")
            checked: !settings.inhibitScreen
            onToggled: settings.inhibitScreen = !checked
        }

        QQC2.CheckBox {
            text: i18nc("@option:check", "Automatically shutting down or sleeping")
            checked: !settings.inhibitSleep
            onToggled: settings.inhibitSleep = !checked
        }

        KQuickControls.KeySequenceItem {
            Kirigami.FormData.label: i18n("Shortcut for switching:")
            keySequence: settings.shortcut
            onCaptureFinished: settings.shortcut = keySequence
        }
    }
}
