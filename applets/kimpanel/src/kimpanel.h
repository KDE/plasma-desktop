/*
 *   Copyright (C) 2009 Wang Hoi <zealot.hoi@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 (or, at
 *   your option, any later version) as published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KIMPANEL_H
#define KIMPANEL_H

#include <plasma/svg.h>
#include <plasma/framesvg.h>
#include <kicon.h>
#include <QString>
#include <QWidget>
#include <QPainter>

#include "kimpaneltype.h"

class PanelAgent;
class StatusBarWidget;
class LookupTableWidget;

class KIMPanel : public QObject {
Q_OBJECT

public:
    KIMPanel(QObject* parent=0);
    ~KIMPanel();

//    void KIMChangeFactory(const QString &uuid);

public slots:
//X     void enable(bool to_enable);
    void execDialog(const Property &prop);
    void execMenu(const QList<Property> &props);

    void showPreedit(bool to_show);
    void showAux(bool to_show);
    void showLookupTable(bool to_show);

    //void exit();
    //void reloadConfig();

    void showConfig();

private:
    PanelAgent *m_panel_agent;
    StatusBarWidget *m_statusbar;
    LookupTableWidget *m_lookup_table;
};
#endif
