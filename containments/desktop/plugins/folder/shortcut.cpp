/*
    SPDX-FileCopyrightText: Ken <https://stackoverflow.com/users/1568857/ken>
    SPDX-FileCopyrightText: 2016 Leslie Zhai <xiangzhai83@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shortcut.h"

#include <KStandardShortcut>

#include <QKeyEvent>

ShortCut::ShortCut(QObject *parent)
    : QObject(parent)
{
}

void ShortCut::installAsEventFilterFor(QObject *target)
{
    if (target) {
        target->installEventFilter(this);
    }
}

bool ShortCut::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        const int keyInt = keyEvent->modifiers() & ~Qt::KeypadModifier | keyEvent->key();
        if (KStandardShortcut::deleteFile().contains(QKeySequence(keyInt))) {
            Q_EMIT deleteFile();
            return true;
        }
        if (KStandardShortcut::renameFile().contains(QKeySequence(keyInt))) {
            Q_EMIT renameFile();
            return true;
        }
        if (KStandardShortcut::moveToTrash().contains(QKeySequence(keyInt))) {
            Q_EMIT moveToTrash();
            return true;
        }
        if (KStandardShortcut::createFolder().contains(QKeySequence(keyInt))) {
            Q_EMIT createFolder();
            return true;
        }
    }

    return QObject::eventFilter(obj, e);
}

#include "moc_shortcut.cpp"
