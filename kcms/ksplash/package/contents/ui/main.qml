/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.1
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.newstuff 1.62 as NewStuff
import org.kde.kcm 1.3 as KCM

KCM.GridViewKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the splash screen theme.")

    KCM.SettingStateBinding {
        configObject: kcm.splashScreenSettings
        settingName: "theme"
        extraEnabledConditions: !kcm.testing
    }

    view.model: kcm.splashSortedModel
    //NOTE: pay attention to never break this binding
    view.currentIndex: kcm.sortModelPluginIndex(kcm.splashScreenSettings.theme)

    // putting the InlineMessage as header item causes it to show up initially despite visible false
    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: testingFailedLabel
            Layout.fillWidth: true
            showCloseButton: true
            type: Kirigami.MessageType.Error

            Connections {
                target: kcm
                function onTestingFailed(processErrorOutput) {
                    testingFailedLabel.text = i18n("Failed to show the splash screen preview.")
                    if (processErrorOutput) {
                        testingFailedLabel.text += "\n" + processErrorOutput
                    }
                    testingFailedLabel.visible = true
                }
                function onError(text) {
                    testingFailedLabel.text = text
                    testingFailedLabel.visible = true
                }
            }
        }
    }

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display
        toolTip: model.description

        thumbnailAvailable: !!model.screenshot
        thumbnail: Image {
            anchors.fill: parent
            source: model.screenshot || ""
            sourceSize: Qt.size(delegate.GridView.view.cellWidth * Screen.devicePixelRatio,
                                delegate.GridView.view.cellHeight * Screen.devicePixelRatio)
            opacity: model.pendingDeletion ? 0.3 : 1
        }
        actions: [
            Kirigami.Action {
                visible: model.pluginName !== "None"
                iconName: "media-playback-start"
                tooltip: i18n("Preview Splash Screen")
                onTriggered: kcm.test(model.pluginName)
            },
            Kirigami.Action {
                iconName: model.pendingDeletion ? "edit-undo" : "edit-delete"
                tooltip: i18n("Uninstall")
                enabled: model.uninstallable
                onTriggered: model.pendingDeletion = !model.pendingDeletion
            }
        ]
        onClicked: {
            kcm.splashScreenSettings.theme = model.pluginName;
            view.forceActiveFocus();
        }
        onDoubleClicked: {
            kcm.save();
        }
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        NewStuff.Button {
            id: newStuffButton
            text: i18n("&Get New Splash Screens…")
            configFile: "ksplash.knsrc"
            viewMode: NewStuff.Page.ViewMode.Preview
            onEntryEvent: function(entry, event) {
                kcm.ghnsEntryChanged(entry);
            }
        }
    }
}
