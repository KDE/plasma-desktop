/*
 *  Copyright (C) 2013 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APP_H
#define APP_H
#include <ibus.h>

#include <QApplication>
#include "panel.h"

class QWidget;
class App : public QApplication {
    Q_OBJECT
public:
    App(int argc, char** argv);
    virtual ~App();
    void finalize();
    virtual bool x11EventFilter(XEvent* event );
private slots:
    void init();
    void clean();
    void grabKey();
    void ungrabKey();
private:
    IBusBus *bus;
    IBusPanelImpanel *impanel;
};

#endif // APP_H
