/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
