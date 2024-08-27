/*
    SPDX-FileCopyrightText: 2024 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    QQC2.CheckBox {
        Kirigami.FormData.label: i18nc("@label", "Shake cursor to find it:")
        text: i18nc("@option check, Enable shake cursor effect", "Enable")

        KCMUtils.SettingStateBinding {
            configObject: kcm.shakeCursorSettings
            settingName: "ShakeCursor"
        }

        checked: kcm.shakeCursorSettings.shakeCursor
        onToggled: kcm.shakeCursorSettings.shakeCursor = checked
    }

    ColumnLayout {
        Kirigami.FormData.label: i18nc("@label Cursor magnification level", "Magnification:")
        Kirigami.FormData.buddyFor: magnificationSlider
        spacing: Kirigami.Units.smallSpacing

        QQC2.Slider {
            id: magnificationSlider

            Layout.preferredWidth: Kirigami.Units.gridUnit * 15

            KCMUtils.SettingStateBinding {
                configObject: kcm.shakeCursorSettings
                settingName: "ShakeCursorMagnification"
                extraEnabledConditions: kcm.shakeCursorSettings.shakeCursor
            }

            from: 2
            to: 10
            stepSize: 1
            snapMode: QQC2.Slider.SnapAlways
            value: kcm.shakeCursorSettings.shakeCursorMagnification
            onMoved: kcm.shakeCursorSettings.shakeCursorMagnification = value
        }

        RowLayout {
            spacing: 0

            QQC2.Label {
                text: i18nc("@label Normal cursor size", "Normal")
                textFormat: Text.PlainText
            }
            Item {
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: i18nc("@label Large cursor size", "Large")
                textFormat: Text.PlainText
            }
        }
    }
}
