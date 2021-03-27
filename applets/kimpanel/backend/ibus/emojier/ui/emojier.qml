/*
 *  Copyright (C) 2019 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.11
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.11 as QQC2
import org.kde.kirigami 2.6 as Kirigami
import org.kde.plasma.emoji 1.0

Kirigami.ApplicationWindow
{
    id: window

    minimumWidth: Math.round(minimumHeight * 1.25)
    minimumHeight: drawer.contentHeight
    width: Kirigami.Units.gridUnit * 25
    height: Kirigami.Units.gridUnit * 25

    EmojiModel {
        id: emoji
    }

    RecentEmojiModel {
        id: recentEmojiModel
    }

    function report(thing, description) {
        if (!visible)
            return;
        console.log("Copied to clipboard:", thing)
        CopyHelper.copyTextToClipboard(thing)
        recentEmojiModel.includeRecent(thing, description);
        window.showPassiveNotification(i18n("%1 copied to the clipboard", thing))
    }

    function startSearch(typedText) {
        window.pageStack.replace("qrc:/ui/CategoryPage.qml", {title: i18n("Search"), category: "", model: emoji, showSearch: true, searchText: typedText})
    }

    onVisibilityChanged: {
        if (visible)
            globalDrawer.actions[recentEmojiModel.count === 0 ? 1 : 0].trigger()
    }

    Kirigami.Action {
        id: recentAction
        checked: window.pageStack.get(0).title === text
        text: i18n("Recent")
        enabled: recentEmojiModel.count > 0

        icon.name: "document-open-recent-symbolic"
        onTriggered: {
            window.pageStack.replace("qrc:/ui/CategoryPage.qml", {title: text, category: "", model: recentEmojiModel, showClearHistoryButton: true})
        }
    }
    Kirigami.Action {
        id: searchAction
        checked: window.pageStack.get(0).title === text
        text: i18n("Search")
        icon.name: "search"
        shortcut: StandardKey.Find

        onTriggered: {
            window.pageStack.replace("qrc:/ui/CategoryPage.qml", {title: text, category: "", model: emoji, showSearch: true })
        }
    }

    CategoryAction {
        id: allAction
        text: i18n("All")
        icon.name: "view-list-icons"
        category: ""
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer

        title: i18n("Categories")
        collapsible: !topContent.activeFocus
        collapsed: true
        modal: false

        header: Kirigami.AbstractApplicationHeader {
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                level: 1
                text: drawer.title
                Layout.fillWidth: true
            }
        }

        Instantiator {
            model: emoji.categories
            CategoryAction {
                category: modelData
            }
            onObjectAdded: {
                var actions = Array.prototype.map.call(drawer.actions, i => i)
                actions.splice(index + 3, 0, object)
                drawer.actions = actions
            }
        }
        actions: [recentAction, searchAction, allAction]
    }
}
