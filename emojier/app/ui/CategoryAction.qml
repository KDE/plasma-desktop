/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import org.kde.kirigami as Kirigami
import org.kde.textaddons.emoticons

Kirigami.Action {
    property string category
    property string categoryId

    checked: window.pageStack.get(0).title === text
    text: i18ndc("org.kde.plasma.emojier", "Emoji Category", category)

    onTriggered: source => {
        window.pageStack.replace(Qt.resolvedUrl("CategoryPage.qml"), {
            title: text,
            category: categoryId,
        });
    }
}
