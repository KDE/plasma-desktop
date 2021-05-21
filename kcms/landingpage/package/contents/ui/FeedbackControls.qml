/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    id: feedbackControlsLayout

    property alias slider: statisticsModeSlider
    readonly property int sliderWidth: Kirigami.Units.gridUnit * 21

    enabled: kcm.feedbackEnabled

    RowLayout {
        QQC2.Slider {
            id: statisticsModeSlider
            readonly property var currentMode: modeOptions[value]
            Layout.fillWidth: true
            Layout.minimumWidth: feedbackControlsLayout.sliderWidth
            Layout.maximumWidth: feedbackControlsLayout.sliderWidth

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
        Layout.maximumWidth: feedbackControlsLayout.sliderWidth
        wrapMode: Text.WordWrap
        text: feedbackController.telemetryName(statisticsModeSlider.currentMode)
    }
}
