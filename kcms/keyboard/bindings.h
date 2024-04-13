/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KActionCollection>

class LayoutUnit;
template<typename T>
class QList;

class KeyboardLayoutActionCollection : public KActionCollection
{
public:
    KeyboardLayoutActionCollection(QObject *parent, bool configAction);
    ~KeyboardLayoutActionCollection() override;

    QAction *getToggleAction();
    QAction *getLastUsedLayoutAction();
    QAction *createLayoutShortcutActon(const LayoutUnit &layoutUnit, int layoutIndex, bool autoload);
    void setLayoutShortcuts(QList<LayoutUnit> &layoutUnits);
    void setToggleShortcut(const QKeySequence &keySequence);
    void setLastUsedLayoutShortcut(const QKeySequence &keySequence);
    void loadLayoutShortcuts(QList<LayoutUnit> &layoutUnits);
    void resetLayoutShortcuts();

private:
    bool configAction;
};
