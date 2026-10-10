/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KActionCollection>

class LayoutUnit;
class UserLayoutModel;

class KeyboardLayoutActionCollection : public KActionCollection
{
public:
    KeyboardLayoutActionCollection(QObject *parent, bool configAction);
    ~KeyboardLayoutActionCollection() override;

    QAction *getToggleAction();
    QAction *getLastUsedLayoutAction();
    QAction *createLayoutShortcutAction(const LayoutUnit &layoutUnit, int layoutIndex, bool autoload);
    void setLayoutShortcuts(const UserLayoutModel *layoutModel);
    void setToggleShortcut(const QKeySequence &keySequence);
    void setLastUsedLayoutShortcut(const QKeySequence &keySequence);
    void loadLayoutShortcuts(UserLayoutModel *layoutModel);
    void resetLayoutShortcuts();

private:
    bool configAction;
};
