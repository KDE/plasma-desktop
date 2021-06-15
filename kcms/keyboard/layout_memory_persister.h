/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUT_MEMORY_PERSISTER_H_
#define LAYOUT_MEMORY_PERSISTER_H_

#include <QString>

#include "x11_helper.h"

class LayoutMemory;
class QFile;

class LayoutMemoryPersister
{
public:
    LayoutMemoryPersister(LayoutMemory &layoutMemory_)
        : layoutMemory(layoutMemory_)
    {
    }

    bool saveToFile(const QFile &file);
    bool restoreFromFile(const QFile &file);

    bool save();
    bool restore();

    LayoutUnit getGlobalLayout() const
    {
        return globalLayout;
    }
    void setGlobalLayout(const LayoutUnit &layout)
    {
        globalLayout = layout;
    }

private:
    LayoutMemory &layoutMemory;
    LayoutUnit globalLayout;

    QString getLayoutMapAsString();

    bool canPersist();
};

#endif /* LAYOUT_MEMORY_PERSISTER_H_ */
