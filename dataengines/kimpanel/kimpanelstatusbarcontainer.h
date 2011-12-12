/***************************************************************************
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KIMPANEL_STATUSBAR_CONTAINER_H
#define KIMPANEL_STATUSBAR_CONTAINER_H

#include "kimpanel/kimpanelagenttype.h"

#include <Plasma/DataContainer>

class PanelAgent;
class KimpanelService;
class KimpanelStatusBarContainer : public Plasma::DataContainer
{
    Q_OBJECT
public:
    KimpanelStatusBarContainer(QObject* parent, PanelAgent* panelAgent);
    Plasma::Service* service(QObject* parent = 0);

protected Q_SLOTS:
    void updateProperty(const KimpanelProperty& property);
    void registerProperties(const QList<KimpanelProperty> &props);

    void execDialog(const KimpanelProperty &prop);
    void execMenu(const QList<KimpanelProperty> &prop_list);
private:
    PanelAgent* m_panelAgent;
    QList< KimpanelProperty > m_props;
};

#endif

