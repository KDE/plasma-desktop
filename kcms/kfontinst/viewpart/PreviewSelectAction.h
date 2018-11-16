#ifndef __PREVIEW_SELECT_ACTION_H__
#define __PREVIEW_SELECT_ACTION_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KSelectAction>
#include "FcEngine.h"

namespace KFI
{

class CPreviewSelectAction : public KSelectAction
{
    Q_OBJECT

    public:

    enum Mode
    {
        Basic,
        BlocksAndScripts,
        ScriptsOnly
    };

    explicit CPreviewSelectAction(QObject *parent, Mode mode=Basic);
    ~CPreviewSelectAction() override { }

    void setStd();
    void setMode(Mode mode);

    Q_SIGNALS:

    void range(const QList<CFcEngine::TRange> &range);

    private Q_SLOTS:

    void selected(int index);

    private:

    int itsNumUnicodeBlocks;
};

}

#endif
