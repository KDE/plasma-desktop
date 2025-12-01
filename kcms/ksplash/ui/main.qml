/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff
import org.kde.kcmutils as KCM

KCM.GridViewKCM {
    id: root

    KCM.SettingStateBinding {
        configObject: kcm.splashScreenSettings
        settingName: "theme"
        extraEnabledConditions: !kcm.testing
    }

    view.model: kcm.splashSortedModel
    //NOTE: pay attention to never break this binding
    view.currentIndex: kcm.sortModelPluginIndex(kcm.splashScreenSettings.theme)

    actions: NewStuff.Action {
        id: newStuffButton
        text: i18n("&Get New…")
        configFile: "ksplash.knsrc"
        viewMode: NewStuff.Page.ViewMode.Preview
        onEntryEvent: function(entry, event) {
            if (event === NewStuff.Entry.StatusChangedEvent) {
                kcm.ghnsEntryChanged(entry);
            }
        }
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: Kirigami.InlineMessage {
        id: testingFailedLabel
        showCloseButton: true
        position: Kirigami.InlineMessage.Position.Header
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

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display
        toolTip: model.description

        thumbnailAvailable: model.screenshot.toString() !== ""
        thumbnail: Image {
            anchors.fill: parent
            source: model.screenshot
            sourceSize: Qt.size(delegate.GridView.view.cellWidth * Screen.devicePixelRatio,
                                delegate.GridView.view.cellHeight * Screen.devicePixelRatio)
            opacity: model.pendingDeletion ? 0.3 : 1
        }
        actions: [
            Kirigami.Action {
                visible: model.pluginName !== "None"
                icon.name: "media-playback-start"
                tooltip: i18n("Preview Splash Screen")
                onTriggered: kcm.test(model.pluginName)
            },
            Kirigami.Action {
                icon.name: model.pendingDeletion ? "edit-undo" : "edit-delete"
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
}
