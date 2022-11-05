/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

Kirigami.FormLayout {
    anchors.left: parent.left
    anchors.right: parent.right

    readonly property bool plasmaPaAvailable: Qt.createComponent("PulseAudio.qml").status === Component.Ready
    readonly property bool plasmoidVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool iconOnly: Plasmoid.pluginName === "org.kde.plasma.icontasks"

    property alias cfg_showToolTips: showToolTips.checked
    property alias cfg_highlightWindows: highlightWindows.checked
    property bool cfg_indicateAudioStreams
    property alias cfg_fill: fill.checked
    property alias cfg_maxStripes: maxStripes.value
    property alias cfg_forceStripes: forceStripes.checked
    property int cfg_iconSpacing: 0

    CheckBox {
        id: showToolTips
        Kirigami.FormData.label: i18n("General:")
        text: i18n("Show small window previews when hovering over Tasks")
    }

    CheckBox {
        id: highlightWindows
        text: i18n("Hide other windows when hovering over previews")
    }

    CheckBox {
        id: indicateAudioStreams
        text: i18n("Mark applications that play audio")
        checked: cfg_indicateAudioStreams && plasmaPaAvailable
        onCheckedChanged: cfg_indicateAudioStreams = checked
        enabled: plasmaPaAvailable
    }

    CheckBox {
        id: fill
        text: i18nc("@option:check", "Fill free space on Panel")
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    SpinBox {
        id: maxStripes
        Kirigami.FormData.label: plasmoidVertical ? i18n("Maximum columns:") : i18n("Maximum rows:")
        from: 1
    }

    CheckBox {
        id: forceStripes
        text: plasmoidVertical ? i18n("Always arrange tasks in rows of as many columns") : i18n("Always arrange tasks in columns of as many rows")
        enabled: maxStripes.value > 1
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    ComboBox {
        visible: iconOnly
        Kirigami.FormData.label: i18n("Spacing between icons:")

        model: [
            {
                "label": i18nc("@item:inlistbox Icon spacing", "Small"),
                "spacing": 0
            },
            {
                "label": i18nc("@item:inlistbox Icon spacing", "Normal"),
                "spacing": 1
            },
            {
                "label": i18nc("@item:inlistbox Icon spacing", "Large"),
                "spacing": 3
            },
        ]

        textRole: "label"
        enabled: !Kirigami.Settings.tabletMode

        currentIndex: {
            if (Kirigami.Settings.tabletMode) {
                return 2; // Large
            }

            switch (cfg_iconSpacing) {
                case 0: return 0; // Small
                case 1: return 1; // Normal
                case 3: return 2; // Large
            }
        }
        onActivated: cfg_iconSpacing = model[currentIndex]["spacing"];
    }

    Label {
        visible: Kirigami.Settings.tabletMode
        text: i18nc("@info:usagetip under a set of radio buttons when Touch Mode is on", "Automatically set to Large when in Touch Mode")
        font: Kirigami.Theme.smallFont
    }
}
