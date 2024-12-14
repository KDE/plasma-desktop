/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Shapes

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

Kirigami.FormLayout {
    id: root

    required property var device

    function reloadOutputView(): void {
        const initialOutputArea = root.device.outputArea;
        if (initialOutputArea === Qt.rect(0, 0, 1, 1)) {
            outputAreaCombo.currentIndex = 0;
        } else if (initialOutputArea.x === 0 && initialOutputArea.y === 0 && initialOutputArea.width === 1) {
            outputAreaCombo.currentIndex = 1;
        } else {
            outputAreaCombo.currentIndex = 2;
        }
        if (outputsCombo.currentIndex !== 0) {
            mapping.setOutputAreaMode(outputAreaCombo.currentIndex);
            mapping.setOutputArea(initialOutputArea);
        }
    }

    QQC2.ComboBox {
        id: outputsCombo
        Kirigami.FormData.label: i18nd("kcm_tablet", "Map to screen:")
        model: OutputsModel {
            id: outputsModel
        }
        currentIndex: {

            if (count === 0) {
                return -1
            }

            outputsModel.rowForDevice(root.device)
        }
        textRole: "display"
        onActivated: index => {
            root.device.outputName = outputsModel.outputNameAt(index)
            root.device.mapToWorkspace = outputsModel.isMapToWorkspaceAt(index)
            // If we are switching to "Follow current screen", ensure we're force switched to "Stretch to Fill"
            if (index === 0) {
                mapping.setOutputAreaMode(0);
                outputAreaCombo.currentIndex = 0;
            }
            root.reloadOutputView();
        }
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18nd("kcm_tablet", "Orientation:")
        model: OrientationsModel {
            id: orientationsModel
        }
        enabled: root.device && root.device.supportsOrientation
        currentIndex: orientationsModel.rowForOrientation(root.device.orientation)
        displayText: root.device.supportsOrientation ? currentText : i18nd("kcm_tablet", "Not Applicable")
        textRole: "display"
        onActivated: {
            root.device.orientation = orientationsModel.orientationAt(currentIndex)
        }
        SettingHighlighter {
            // First index is "Default"
            highlight: root.device.supportsOrientation && parent.currentIndex !== 0
        }
    }

    QQC2.ComboBox {
        id: outputAreaCombo
        Layout.fillWidth: true
        Kirigami.FormData.label: i18nd("kcm_tablet", "Mapped Area:")
        enabled: outputsCombo.currentIndex !== 0
        model: outputsCombo.currentIndex === 0 ?
            // We don't support mapping to a portion of the screen with follow current screen, since we have no way to visualize that
            [i18nc("@item:inlistbox Stretch and fill to the screen", "Stretch and Fill")] :
            [i18nc("@item:inlistbox Stretch and fill to the screen", "Stretch and Fill"),
                i18nc("@item:inlistbox Keep aspect ratio and fit within the screen", "Keep Aspect Ratio and Fit"),
                i18nc("@item:inlistbox Map to portion of the screen", "Map to Portion")]
        onActivated: index => {
            mapping.changed = true;
            mapping.setOutputAreaMode(index);
        }
        SettingHighlighter {
            // The default is stretch to screen
            highlight: outputAreaCombo.currentIndex !== 0
        }
    }

    // Mapping a tablet onto another display
    ExternalScreenMapping {
        id: mapping

        visible: root.device && outputsCombo.currentIndex !== 0
        device: root.device
        mode: outputAreaCombo.currentIndex

        Layout.fillWidth: true
    }
}
