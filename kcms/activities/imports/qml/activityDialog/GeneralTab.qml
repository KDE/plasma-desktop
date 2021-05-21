/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Kirigami.FormLayout {
    id: root

    function setFocus() {
        activityName.forceActiveFocus();
    }

    property string activityId: ""

    property alias activityName        : activityName.text
    property alias activityDescription : activityDescription.text
    property alias activityIcon        : activityIcon.icon.name
    property alias activityIsPrivate   : activityIsPrivate.checked
    property alias activityShortcut    : activityShortcut.keySequence

    Item {
        height: Kirigami.Units.smallSpacing
    }

    QQC2.Button {
        id: activityIcon
        implicitHeight:  Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
        implicitWidth: height
        icon.height: Kirigami.Units.iconSizes.medium
        icon.width: Kirigami.Units.iconSizes.medium
        icon.name: "activities"
        Kirigami.FormData.label: i18nd("kcm_activities5", "Icon:")

        KQuickControlsAddons.IconDialog {
            id: iconDialog
            onIconNameChanged: activityIcon.icon.name = iconName
        }

        onClicked: {
            iconDialog.open();
        }
    }

    QQC2.TextField {
        id: activityName
        Kirigami.FormData.label: i18nd("kcm_activities5", "Name:")
    }

    QQC2.TextField {
        id: activityDescription
        Kirigami.FormData.label: i18nd("kcm_activities5", "Description:")
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    QQC2.CheckBox {
        id: activityIsPrivate
        Kirigami.FormData.label: i18nd("kcm_activities5", "Privacy:")
        text: i18nd("kcm_activities5", "Do not track usage for this activity")
    }

    KQuickControls.KeySequenceItem {
        id: activityShortcut
        Kirigami.FormData.label: i18nd("kcm_activities5", "Shortcut for switching:")
    }
}
