/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "menuhelper.h"

#include <QAction>
#include <QMenu>

MenuHelper::MenuHelper(QObject *parent)
    : QObject(parent)
{
}

MenuHelper::~MenuHelper()
{
}

QString MenuHelper::iconName(QAction *action) const
{
    if (!action) {
        return QString();
    }

    return action->icon().name();
}

void MenuHelper::setMenu(QAction *action, QObject *menu)
{
    QMenu *bla = qobject_cast<QMenu *>(menu);

    if (action && bla) {
        action->setMenu(bla);
    }
}
