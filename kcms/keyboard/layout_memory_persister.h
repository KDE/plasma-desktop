/*
 *  Copyright (C) 2011 Andriy Rysin (rysin@kde.org)
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

#ifndef LAYOUT_MEMORY_PERSISTER_H_
#define LAYOUT_MEMORY_PERSISTER_H_

#include <QString>

#include "x11_helper.h"

class LayoutMemory;
class QFile;

class LayoutMemoryPersister {
public:
	LayoutMemoryPersister(LayoutMemory& layoutMemory_):
		layoutMemory(layoutMemory_) {}

	bool saveToFile(const QFile& file);
	bool restoreFromFile(const QFile& file);

	bool save();
	bool restore();

	LayoutUnit getGlobalLayout() const { return globalLayout; }
	void setGlobalLayout(const LayoutUnit& layout) { globalLayout = layout; }

private:
	LayoutMemory& layoutMemory;
    LayoutUnit globalLayout;

	QString getLayoutMapAsString();

	bool canPersist();
};

#endif /* LAYOUT_MEMORY_PERSISTER_H_ */
