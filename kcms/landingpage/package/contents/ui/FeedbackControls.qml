/*
 * Copyright 2021 Marco Martin <mart@kde.org>
 * Copyright 2018 Furkan Tokac <furkantokac34@gmail.com>
 * Copyright (C) 2019 Nate Graham <nate@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.6 as KCM

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.userfeedback 1.0 as UserFeedback

ColumnLayout {
    property alias slider: statisticsModeSlider

    enabled: kcm.feedbackEnabled

    RowLayout {
        QQC2.Slider {
            id: statisticsModeSlider
            readonly property var currentMode: modeOptions[value]
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 21
            Layout.maximumWidth: Kirigami.Units.gridUnit * 21

            readonly property var modeOptions: [UserFeedback.Provider.NoTelemetry, UserFeedback.Provider.BasicSystemInformation, UserFeedback.Provider.BasicUsageStatistics,
                                                UserFeedback.Provider.DetailedSystemInformation, UserFeedback.Provider.DetailedUsageStatistics]
            from: 0
            to: modeOptions.length - 1
            stepSize: 1
            snapMode: QQC2.Slider.SnapAlways

            function findIndex(array, what, defaultValue) {
                for (var v in array) {
                    if (array[v] == what)
                        return v;
                }
                return defaultValue;
            }

            value: findIndex(modeOptions, kcm.feedbackSettings.feedbackLevel, 0)

            onMoved: {
                kcm.feedbackSettings.feedbackLevel = modeOptions[value]
            }

            KCM.SettingStateBinding {
                configObject: kcm.feedbackSettings
                settingName: "feedbackLevel"
                extraEnabledConditions: kcm.feedbackEnabled
            }
            UserFeedback.FeedbackConfigUiController {
                id: feedbackController
                applicationName: i18n("Plasma")
            }
        }
        KCM.ContextualHelpButton {
            toolTipText: {
                let description = i18n("You can help KDE improve Plasma by contributing information on how you use it, so we can focus on things that matter to you.<nl/><nl/>Contributing this information is optional and entirely anonymous. We never collect your personal data, files you use, websites you visit, or information that could identify you.");

                if (kcm.feedbackSettings.feedbackLevel === UserFeedback.Provider.NoTelemetry) {
                    return "<h4>" + i18n("No data will be sent.") + "</h4><nl/><p>" + description + "</p>";
                }
                let text = "<p>" + description + "</p><nl/><h4>" + i18n("The following information will be sent:") + "</h4><ul>";
                for (let i in kcm.feedbackSources) {
                    text += "<li>" + kcm.feedbackSources[i].description + "</li>";
                }
                text += "</ul>";
                return text;
            }
        }
    }
    QQC2.Label {
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: Kirigami.Units.gridUnit * 21
        wrapMode: Text.WordWrap
        text: feedbackController.telemetryName(statisticsModeSlider.currentMode)
    }
}
