#ifndef __VIEWER_H__
#define __VIEWER_H__

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

#include <KParts/ReadOnlyPart>
#include <KParts/MainWindow>

class QAction;
class QUrl;

namespace KFI
{

class CViewer : public KParts::MainWindow
{
    Q_OBJECT

    public:

    CViewer();
    ~CViewer() override { }
    void showUrl(const QUrl &url);

    public Q_SLOTS:

    void fileOpen();
    void configureKeys();
    void enableAction(const char *name, bool enable);

    private:

    KParts::ReadOnlyPart *itsPreview;
    QAction              *itsPrintAct;
};

}

#endif
