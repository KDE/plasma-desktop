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

#ifndef KIMPANEL_INPUTPANEL_CONTAINER_H
#define KIMPANEL_INPUTPANEL_CONTAINER_H

#include "kimpanel/kimpanelagenttype.h"

#include <Plasma/DataContainer>

class PanelAgent;
class KimpanelService;
class KimpanelInputPanelContainer : public Plasma::DataContainer
{
    Q_OBJECT
public:
    KimpanelInputPanelContainer(QObject* parent, PanelAgent* panelAgent);
    Plasma::Service* service(QObject* parent = 0);

protected Q_SLOTS:
    void updatePreeditText(const QString& text, const QList<TextAttribute>& attrList);
    void updateAux(const QString& text, const QList<TextAttribute>& attrList);
    void updatePreeditCaret(int pos);
    void updateLookupTable(const KimpanelLookupTable& lookupTable);
    void updateLookupTableFull(const KimpanelLookupTable& lookupTable,int cursor, int layout);
    void updateSpotLocation(int x, int y);
    void updateSpotRect(int x, int y, int w, int h);
    void showAux(bool visible);
    void showPreedit(bool visible);
    void showLookupTable(bool visible);
    void updateLookupTableCursor(int cursor);
private:
    PanelAgent* m_panelAgent;
};

#endif

