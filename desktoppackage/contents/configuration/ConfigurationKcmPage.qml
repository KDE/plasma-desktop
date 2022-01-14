/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Controls 2.2 as QQC2
import org.kde.kirigami 2.5 as Kirigami

Kirigami.Page {
    id: container

    required property QtObject kcm
    required property Item internalPage

    signal settingValueChanged()

    title: kcm.name
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0
    flickable: internalPage.flickable
    actions.main: internalPage.actions.main
    actions.contextualActions: internalPage.contextualActions

    onInternalPageChanged: {
        internalPage.parent = contentItem;
        internalPage.anchors.fill = contentItem;
    }
    onActiveFocusChanged: {
        if (activeFocus) {
            internalPage.forceActiveFocus();
        }
    }

    Component.onCompleted: {
        kcm.load()
    }

    function saveConfig() {
        kcm.save();
    }

    data: [
        Connections {
            target: kcm
            onPagePushed: {
                app.pageStack.push(configurationKcmPageComponent.createObject(app.pageStack, {"kcm": kcm, "internalPage": page}));
            }
            onPageRemoved: app.pageStack.pop();
        },
        Connections {
            target: app.pageStack
            onPageRemoved: {
                if (kcm.needsSave) {
                    kcm.save()
                }
                if (page == container) {
                    page.destroy();
                }
            }
        }
    ]
    Connections {
        target: kcm
        function onNeedsSaveChanged() {
            if (kcm.needsSave) {
                container.settingValueChanged()
            }
        }
    }
}
