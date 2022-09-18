/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import org.kde.kirigami 2.6 as Kirigami

Kirigami.Action {
    function getIcon(category) {
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

    property string category
    checked: window.pageStack.get(0).title === text
    text: i18ndc("org.kde.plasma.emojier", "Emoji Category", category)

    icon.name: getIcon(category)
    onTriggered: {
        window.pageStack.replace("qrc:/ui/CategoryPage.qml", {title: text, category: category, model: emoji })
    }
}
