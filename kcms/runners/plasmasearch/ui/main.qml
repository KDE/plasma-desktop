/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.1

import org.kde.config
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.19 as Kirigami
import org.kde.newstuff 1.91 as NewStuff

KCM.ScrollViewKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 18

    header: ColumnLayout {
        QQC2.Label {
            text: i18n("Enable or disable plugins (used in KRunner, Application Launcher, and the Overview effect)")
        }
        Kirigami.SearchField {
            Layout.fillWidth: true
            id: searchField
            wrapMode: Text.WordWrap
        }
    }

    view: KCM.PluginSelector {
        sourceModel: kcm.model
        query: searchField.text
        delegate: KCM.PluginDelegate {
            onConfigTriggered: kcm.showKCM(model.config, [], model.metaData)
            highlighted: false
            hoverEnabled: false
        }
    }

    footer: Kirigami.ActionToolBar {
        flat: false
        alignment: Qt.AlignRight
        actions: [
            Kirigami.Action {
                text: i18n("Configure KRunner…")
                onTriggered: kcm.showKRunnerKCM()
            },
            NewStuff.Action {
                text: i18n("Get New Plugins…")
                visible: KAuthorized.authorize(KAuthorized.GHNS)
                configFile: "krunner.knsrc"
                onEntryEvent: function (entry, event) {
                    if (event == NewStuff.Entry.StatusChangedEvent) {
                        kcm.reloadPlugin()
                    }
                }
            } 
        ]
    }
}
