/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.config as KConfig
import org.kde.kquickcontrolsaddons as KQuickControlsAddons

import org.kde.textaddons.emoticons

Kirigami.ApplicationWindow {
    id: window

    minimumWidth: Math.round(minimumHeight * 1.25)
    width: Kirigami.Units.gridUnit * 25
    height: Kirigami.Units.gridUnit * 25

    KConfig.WindowStateSaver {
        configGroupName: "MainWindow"
    }

    /*RecentEmojiModel {
        id: recentEmojiModel
    }*/

    function report(thing: string, description: string): void {
        if (!visible) {
            return;
        }
        clipboard.content = thing;
        //recentEmojiModel.includeRecent(thing, description);
        window.showPassiveNotification(i18n("%1 copied to the clipboard", thing))
    }

    KQuickControlsAddons.Clipboard { id: clipboard }

    /*Kirigami.Action {
        id: recentAction
        checked: window.pageStack.get(0).title === text
        text: i18n("Recent")

        icon.name: "document-open-recent-symbolic"
        onTriggered: source => {
            window.pageStack.replace(Qt.resolvedUrl("CategoryPage.qml"), {
                title: text,
                category: "",
                model: recentEmojiModel,
                showClearHistoryButton: true,
            });
        }
    }*/

    Kirigami.Action {
        id: searchAction
        checked: window.pageStack.get(0).title === text
        text: i18n("Search")
        icon.name: "search"
        shortcut: StandardKey.Find

        onTriggered: source => {
            window.pageStack.replace(Qt.resolvedUrl("CategoryPage.qml"), {
                title: text,
                category: "",
                model: emoji,
                showSearch: true,
            });
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
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    level: 1
                    elide: Text.ElideRight
                    text: drawer.title
                    textFormat: Text.PlainText
                }
            }

            function getIcon(category: string): string {
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
                model: UnicodeEmoticonManager.categories

                CategoryAction {
                    category: modelData.i18nName
                    icon.name: getIcon(modelData.category)
                    categoryId: modelData.category
                }

                onObjectAdded: (index, object) => {
                    if (++loadCount !== model.length) {
                        return;
                    }

                    const actions = [searchAction, allAction];
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
        //recentAction.trigger();

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
