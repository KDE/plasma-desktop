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
        CopyHelper.copyTextToClipboard(thing)
        recentEmojiModel.includeRecent(thing, description);
        window.showPassiveNotification(i18n("%1 copied to the clipboard", thing))
    }

    Kirigami.Action {
        id: recentAction
        checked: window.pageStack.get(0).title === text
        text: i18n("Recent")

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

    Component {
        id: drawerComponent

        Kirigami.GlobalDrawer {
            id: drawer

            title: i18n("Categories")
            collapsible: !topContent.activeFocus
            collapsed: true
            modal: false

            header: Kirigami.AbstractApplicationHeader {
                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing
                leftPadding: Kirigami.Units.gridUnit
                rightPadding: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    level: 1
                    text: drawer.title
                    Layout.fillWidth: true
                }
            }

            function getIcon(category: string) {
                switch (category.trim()) {
                    case 'Activities': return 'games-highscores'
                    case 'Animals and Nature': return 'animal'
                    case 'Flags': return 'flag'
                    case 'Food and Drink': return 'food'
                    case 'Objects': return 'object'
                    case 'People and Body': return 'user'
                    case 'Smileys and Emotion': return 'smiley'
                    case 'Symbols': return 'checkmark'
                    case 'Travel and Places': return 'globe'
                    default: return 'folder'
                }
            }

            Instantiator {
                id: instantiator
                property int loadCount: 0
                asynchronous: true
                model: emoji.categories
                CategoryAction {
                    category: modelData
                    icon.name: drawer.getIcon(category)
                }
                onObjectAdded: (index, object) => {
                    if (++loadCount !== model.length) {
                        return;
                    }

                    let actions = [recentAction, searchAction, allAction];
                    for (let i = 0; i < count; ++i) {
                        actions.push(this.objectAt(i));
                    }
                    drawer.actions = actions;
                    // The extra gridUnit is to account for the header that appears when expanded
                    window.minimumHeight = Qt.binding(() => drawer.contentHeight + Kirigami.Units.gridUnit * 2);

                }
            }
        }
    }

    Component.onCompleted: {
        recentAction.trigger();

        const incubator = drawerComponent.incubateObject(window);
        if (incubator.status !== Component.Ready) {
            incubator.onStatusChanged = function(status) {
                if (status === Component.Ready) {
                    window.globalDrawer = incubator.object;
                }
            };
        } else {
            window.globalDrawer = incubator.object;
        }
    }
}
