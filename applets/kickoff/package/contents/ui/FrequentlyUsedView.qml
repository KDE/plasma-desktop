/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0

import org.kde.plasma.private.kicker 0.1 as Kicker

BaseView {
    objectName: "FrequentlyUsedView"

    model: Kicker.RecentUsageModel {
        favoritesModel: globalFavorites
        ordering: 1 // Popular / Frequently Used
    }
}
