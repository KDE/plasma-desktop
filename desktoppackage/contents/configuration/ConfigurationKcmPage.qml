/*
 *  Copyright 2015 Marco Martin <mart@kde.org>
 *  Copyright 2020 Nicolas Fella <nicolas.fella@gmx.de>
 *  Copyright 2020 Carl Schwan <carlschwan@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.6
import QtQuick.Controls 2.2 as Controls
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
