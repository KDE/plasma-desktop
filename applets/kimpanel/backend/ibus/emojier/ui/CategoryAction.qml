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

import org.kde.kirigami 2.6 as Kirigami

Kirigami.Action {
    function getIcon(category) {
        switch (category.trim()) {
            case 'Activities': return 'games-highscores'
            case 'Animals & Nature': return 'animal'
            case 'Flags': return 'flag'
            case 'Food & Drink': return 'food'
            case 'Objects': return 'object'
            case 'People & Body': return 'user'
            case 'Smileys & Emotion':
            case 'Smileys & People': return 'smiley'
            case 'Symbols': return 'love'
            case 'Travel & Places': return 'globe'
            default: return 'folder'
        }
    }

    property string category
    checked: window.pageStack.get(0).title === text
    text: i18nd("ibus10", category) //Get the translation of emoji categories from ibus10.mo installed by ibus

    icon.name: getIcon(category)
    onTriggered: {
        window.pageStack.replace("qrc:/ui/CategoryPage.qml", {title: text, category: category, model: emoji })
    }
}
