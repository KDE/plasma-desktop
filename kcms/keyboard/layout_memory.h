/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef LAYOUT_MEMORY_H_
#define LAYOUT_MEMORY_H_

#include <QString>
#include <QMap>
#include <QWidgetList> //For WId

#include "x11_helper.h"
#include "keyboard_config.h"

class LayoutMemoryPersister;

class LayoutMemory : public QObject
{
    Q_OBJECT

    // if there's some transient windows coming up we'll need to either ignore it
    // or in case of layout switcher popup menu to apply new layout to previous key
    QString previousLayoutMapKey;
    QList<LayoutUnit> prevLayoutList;
    const KeyboardConfig& keyboardConfig;

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
	LayoutMemory(const KeyboardConfig& keyboardConfig);
	~LayoutMemory() override;

	void configChanged();

protected:
    //QVariant does not support long for WId so we'll use QString for key instead
    QMap<QString, LayoutSet> layoutMap;

	friend class LayoutMemoryPersister;
};

#endif /* LAYOUT_MEMORY_H_ */
