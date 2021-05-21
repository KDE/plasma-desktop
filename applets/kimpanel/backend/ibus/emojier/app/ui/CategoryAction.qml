/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
