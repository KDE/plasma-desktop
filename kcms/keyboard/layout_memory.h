/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUT_MEMORY_H_
#define LAYOUT_MEMORY_H_

#include <QMap>
#include <QString>
#include <QWidgetList> //For WId

#include "keyboard_config.h"
#include "x11_helper.h"

class LayoutMemoryPersister;

class LayoutMemory : public QObject
{
    Q_OBJECT

    // if there's some transient windows coming up we'll need to either ignore it
    // or in case of layout switcher popup menu to apply new layout to previous key
    QString previousLayoutMapKey;
    QList<LayoutUnit> prevLayoutList;
    const KeyboardConfig &keyboardConfig;

    void registerListeners();
    void unregisterListeners();
    QString getCurrentMapKey();
    void setCurrentLayoutFromMap();

public Q_SLOTS:
    void layoutMapChanged();
    void layoutChanged();
    void windowChanged(WId wId);
    void desktopChanged(int desktop);

public:
    LayoutMemory(const KeyboardConfig &keyboardConfig);
    ~LayoutMemory() override;

    void configChanged();

protected:
    // QVariant does not support long for WId so we'll use QString for key instead
    QMap<QString, LayoutSet> layoutMap;

    friend class LayoutMemoryPersister;
};

#endif /* LAYOUT_MEMORY_H_ */
