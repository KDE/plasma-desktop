/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kconfig 1.0 // for KAuthorized
import org.kde.kcm 1.1 as KCM

KCM.GridViewKCM {
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the splash screen theme.")

    view.model: kcm.splashModel
    //NOTE: pay attention to never break this binding
    view.currentIndex: kcm.selectedPluginIndex
    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display
        toolTip: model.description

        thumbnailAvailable: !!model.screenshot
        thumbnail: Image {
            anchors.fill: parent
            source: model.screenshot || ""
        }
        actions: [
            Kirigami.Action {
                visible: model.pluginName != "None"
                iconName: "media-playback-start"
                tooltip: i18n("Test Splash Screen")
                onTriggered: kcm.test(model.pluginName)
            }
        ]
        onClicked: {
            kcm.selectedPlugin = model.pluginName;
            view.forceActiveFocus();
        }
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        QtControls.Button {
            iconName: "get-hot-new-stuff"
            text: i18n("&Get New Splash Screens...")
            onClicked: kcm.getNewClicked();
            visible: KAuthorized.authorize("ghns")
        }
    }
}
