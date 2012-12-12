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

#include "kimpanelinputpanelcontainer.h"
#include "kimpanelservice.h"
#include "kimpanelagent.h"
#include "kimpaneldataengine.h"

KimpanelInputPanelContainer::KimpanelInputPanelContainer(QObject* parent, PanelAgent* panelAgent):
    DataContainer(parent),
    m_panelAgent(panelAgent)
{
    connect(m_panelAgent, SIGNAL(updateAux(QString, QList<TextAttribute>)), this, SLOT(updateAux(QString, QList<TextAttribute>)));
    connect(m_panelAgent, SIGNAL(updatePreeditText(QString, QList<TextAttribute>)), this, SLOT(updatePreeditText(QString, QList<TextAttribute>)));
    connect(m_panelAgent, SIGNAL(updatePreeditCaret(int)), this, SLOT(updatePreeditCaret(int)));
    connect(m_panelAgent, SIGNAL(updateLookupTable(KimpanelLookupTable)), this, SLOT(updateLookupTable(KimpanelLookupTable)));
    connect(m_panelAgent, SIGNAL(updateSpotLocation(int,int)), this, SLOT(updateSpotLocation(int,int)));
    connect(m_panelAgent, SIGNAL(updateSpotRect(int, int, int, int)), this, SLOT(updateSpotRect(int, int, int, int)));
    connect(m_panelAgent, SIGNAL(showAux(bool)), this, SLOT(showAux(bool)));
    connect(m_panelAgent, SIGNAL(showPreedit(bool)), this, SLOT(showPreedit(bool)));
    connect(m_panelAgent, SIGNAL(showLookupTable(bool)), this, SLOT(showLookupTable(bool)));
    connect(m_panelAgent, SIGNAL(updateLookupTableCursor(int)), this, SLOT(updateLookupTableCursor(int)));
    connect(m_panelAgent, SIGNAL(updateLookupTableFull(KimpanelLookupTable,int,int)), this, SLOT(updateLookupTableFull(KimpanelLookupTable,int,int)));
}

Plasma::Service* KimpanelInputPanelContainer::service(QObject* parent)
{
    KimpanelService *controller = new KimpanelService(parent, INPUTPANEL_SOURCE_NAME, m_panelAgent);
    connect(this, SIGNAL(updateRequested(DataContainer*)),
            controller, SLOT(enableKimpanelOperations()));
    return controller;
}

void KimpanelInputPanelContainer::updateAux(const QString& text, const QList< TextAttribute >& attrList)
{
    Q_UNUSED(attrList);
    setData("AuxText", text);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updatePreeditText(const QString& text, const QList<TextAttribute>& attrList)
{
    Q_UNUSED(attrList);
    setData("PreeditText", text);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updatePreeditCaret(int pos)
{
    setData("CaretPos", pos);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateLookupTable(const KimpanelLookupTable& lookupTable)
{
    QVariantList candidateList;
    Q_FOREACH(const KimpanelLookupTable::Entry & entry, lookupTable.entries) {
        QVariantMap map;
        map["label"] = entry.label;
        map["text"] = entry.text;
        candidateList += map;
    }
    setData("LookupTable", candidateList);
    setData("HasPrev", lookupTable.has_prev);
    setData("HasNext", lookupTable.has_next);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateSpotLocation(int x, int y)
{
    updateSpotRect(x, y, 0, 0);
}

void KimpanelInputPanelContainer::updateSpotRect(int x, int y, int w, int h)
{
    setData("Position", QRect(x, y, w, h));
    checkForUpdate();
}

void KimpanelInputPanelContainer::showAux(bool visible)
{
    setData("AuxVisible", visible);
    checkForUpdate();
}

void KimpanelInputPanelContainer::showPreedit(bool visible)
{
    setData("PreeditVisible", visible);
    checkForUpdate();
}

void KimpanelInputPanelContainer::showLookupTable(bool visible)
{
    setData("LookupTableVisible", visible);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateLookupTableCursor(int cursor)
{
    setData("LookupTableCursor", cursor);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateLookupTableFull(const KimpanelLookupTable& lookupTable, int cursor, int layout)
{
    QVariantList candidateList;
    Q_FOREACH(const KimpanelLookupTable::Entry & entry, lookupTable.entries) {
        QVariantMap map;
        map["label"] = entry.label;
        map["text"] = entry.text;
        candidateList += map;
    }
    setData("LookupTable", candidateList);
    setData("HasPrev", lookupTable.has_prev);
    setData("HasNext", lookupTable.has_next);
    setData("LookupTableCursor", cursor);
    setData("LookupTableLayout", layout);
    checkForUpdate();

}