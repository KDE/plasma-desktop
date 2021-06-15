/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BINDINGS_H_
#define BINDINGS_H_

#include <KActionCollection>

struct Rules;
class LayoutUnit;
template<typename T>
class QList;

class KeyboardLayoutActionCollection : public KActionCollection
{
public:
    KeyboardLayoutActionCollection(QObject *parent, bool configAction);
    ~KeyboardLayoutActionCollection() override;

    QAction *getToggleAction();
    QAction *createLayoutShortcutActon(const LayoutUnit &layoutUnit, int layoutIndex, const Rules *rules, bool autoload);
    void setLayoutShortcuts(QList<LayoutUnit> &layoutUnits, const Rules *rules);
    void setToggleShortcut(const QKeySequence &keySequence);
    void loadLayoutShortcuts(QList<LayoutUnit> &layoutUnits, const Rules *rules);
    void resetLayoutShortcuts();

private:
    bool configAction;
};

#endif /* BINDINGS_H_ */
