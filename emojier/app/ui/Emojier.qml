/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.config as KConfig

import org.kde.plasma.emoji

Kirigami.ApplicationWindow {
    id: window

    minimumWidth: Math.round(minimumHeight * 1.25)
    // Correct height required for no scrolling — drawer's header's
    // implicit height is used instead of height, so add the difference
    minimumHeight: drawer.contentHeight + (drawer.header.height - drawer.header.implicitHeight)

    width: Kirigami.Units.gridUnit * 25
    height: Kirigami.Units.gridUnit * 25

    KConfig.WindowStateSaver {
        configGroupName: "MainWindow"
    }

    EmojiModel {
        id: emoji
    }

    RecentEmojiModel {
        id: recentEmojiModel
    }

    function report(thing: string, description: string): void {
        if (!visible) {
            return;
        }
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
            window.pageStack.replace(Qt.resolvedUrl("CategoryPage.qml"), {
                title: text,
                category: "",
                model: recentEmojiModel,
                showClearHistoryButton: true,
            });
        }
    }

    Kirigami.Action {
        id: allAction
        checked: window.pageStack.get(0).title === text
        text: i18nc("@title:page All emojis", "All")
        icon.name: "view-list-icons"

        onTriggered: {
            window.pageStack.replace(Qt.resolvedUrl("CategoryPage.qml"), {
                title: text,
                category: "",
                model: emoji,
            });
        }
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer

        collapsible: !topContent.activeFocus
        showHeaderWhenCollapsed: true
        collapseButtonVisible: false
        collapsed: true
        modal: false

        actions: [recentAction, allAction]

        header: Kirigami.AbstractApplicationHeader {
            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing

            QQC2.ToolButton {
                anchors.left: parent.left
                anchors.right: parent.right
                text: drawer.collapsed ? "" : qsTr("Close Sidebar")
                icon.name: {
                    if (drawer.collapsed) {
                        return Application.layoutDirection === Qt.RightToLeft ? "sidebar-expand-right" : "sidebar-expand-left";
                    } else {
                        return Application.layoutDirection === Qt.RightToLeft ? "sidebar-collapse-right" : "sidebar-collapse-left";
                    }
                }

                onClicked: drawer.collapsed = !drawer.collapsed

                QQC2.ToolTip.visible: drawer.collapsed && (Kirigami.Settings.tabletMode ? pressed : hovered)
                QQC2.ToolTip.text: qsTr("Open Sidebar")
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
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

            model: emoji.categories
            delegate: Kirigami.Action {
                required property string modelData

                checked: window.pageStack.get(0).title === text
                icon.name: drawer.getIcon(modelData)
                text: i18ndc("org.kde.plasma.emojier", "Emoji Category", modelData)

                onTriggered: {
                    window.pageStack.replace(Qt.resolvedUrl("CategoryPage.qml"), {
                        title: text,
                        category: modelData,
                        model: emoji,
                    });
                }
            }

            onObjectAdded: (index, object) => {
                drawer.actions.push(object);
            }
        }
    }

    Component.onCompleted: {
        if (recentEmojiModel.count > 0) {
            recentAction.trigger();
        } else {
            allAction.trigger();
        }
    }
}
