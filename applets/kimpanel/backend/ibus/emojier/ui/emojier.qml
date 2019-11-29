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
    title: i18n("Emoji Picker")

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
        visible = false
    }

    onVisibilityChanged: {
        if (visible)
            globalDrawer.actions[recentEmojiModel.count === 0 ? 1 : 0].trigger()
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer
        title: i18n("Categories")
        collapsible: !topContent.activeFocus
        collapsed: true
        modal: false

        function createCategoryActions(categories) {
            var actions = []
            for(var i in categories) {
                var cat = categories[i];
                var catAction = categoryActionComponent.createObject(drawer, { category: cat });
                actions.push(catAction)
            }
            return actions;
        }

        actions: createCategoryActions(emoji.categories)

        Component {
            id: categoryActionComponent
            Kirigami.Action {
                readonly property bool isRecent: category === ":recent:"
                property string category
                checked: window.pageStack.get(0).title === text
                text: category.length === 0 ? i18n("All")
                        : isRecent          ? i18n("Recent")
                        : category.replace(/&/g, "&&");
                enabled: !isRecent || recentEmojiModel.count > 0

                icon.name: isRecent ? "document-open-recent-symbolic"
                         : category.length === 0 ? "view-list-icons"
                         : "image://text/" + emoji.findFirstEmojiForCategory(category)
                onTriggered: {
                    window.pageStack.replace("qrc:/ui/CategoryPage.qml", {title: text, category: isRecent ? "" : category, model: isRecent ? recentEmojiModel : emoji })
                }
            }
        }
    }
}
