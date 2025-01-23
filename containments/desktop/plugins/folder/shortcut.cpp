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
    switch (e->type()) {
    case QEvent::ShortcutOverride: {
        // GridView intercepts the cut/copy/paste shortcuts, but then it does nothing with them
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        const int keyInt = keyEvent->modifiers() & ~Qt::KeypadModifier | keyEvent->key();
        if (KStandardShortcut::copy().contains(QKeySequence(keyInt))) {
            Q_EMIT copyFile();
            return true;
        }
        if (KStandardShortcut::paste().contains(QKeySequence(keyInt))) {
            Q_EMIT pasteFile();
            return true;
        }
        break;
    }
    case QEvent::KeyPress: {
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
        if (KStandardShortcut::cut().contains(QKeySequence(keyInt))) {
            Q_EMIT cutFile();
            return true;
        }
    }
    default:
        break;
    }

    return QObject::eventFilter(obj, e);
}

#include "moc_shortcut.cpp"
