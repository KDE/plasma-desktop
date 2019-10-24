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
    connect(m_panelAgent, &PanelAgent::updateAux, this, &KimpanelInputPanelContainer::updateAux);
    connect(m_panelAgent, &PanelAgent::updatePreeditText, this, &KimpanelInputPanelContainer::updatePreeditText);
    connect(m_panelAgent, &PanelAgent::updatePreeditCaret, this, &KimpanelInputPanelContainer::updatePreeditCaret);
    connect(m_panelAgent, &PanelAgent::updateLookupTable, this, &KimpanelInputPanelContainer::updateLookupTable);
    connect(m_panelAgent, &PanelAgent::updateSpotLocation, this, &KimpanelInputPanelContainer::updateSpotLocation);
    connect(m_panelAgent, &PanelAgent::updateSpotRect, this, &KimpanelInputPanelContainer::updateSpotRect);
    connect(m_panelAgent, &PanelAgent::showAux, this, &KimpanelInputPanelContainer::showAux);
    connect(m_panelAgent, &PanelAgent::showPreedit, this, &KimpanelInputPanelContainer::showPreedit);
    connect(m_panelAgent, &PanelAgent::showLookupTable, this, &KimpanelInputPanelContainer::showLookupTable);
    connect(m_panelAgent, &PanelAgent::updateLookupTableCursor, this, &KimpanelInputPanelContainer::updateLookupTableCursor);
    connect(m_panelAgent, &PanelAgent::updateLookupTableFull, this, &KimpanelInputPanelContainer::updateLookupTableFull);
}

Plasma::Service* KimpanelInputPanelContainer::service(QObject* parent)
{
    KimpanelService *controller = new KimpanelService(parent, QLatin1String(INPUTPANEL_SOURCE_NAME), m_panelAgent);
    connect(this, &Plasma::DataContainer::updateRequested,
            controller, &KimpanelService::enableKimpanelOperations);
    return controller;
}

void KimpanelInputPanelContainer::updateAux(const QString& text, const QList< TextAttribute >& attrList)
{
    Q_UNUSED(attrList);
    setData(QStringLiteral("AuxText"), text);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updatePreeditText(const QString& text, const QList<TextAttribute>& attrList)
{
    Q_UNUSED(attrList);
    setData(QStringLiteral("PreeditText"), text);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updatePreeditCaret(int pos)
{
    setData(QStringLiteral("CaretPos"), pos);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateLookupTable(const KimpanelLookupTable& lookupTable)
{
    QVariantList candidateList;
    Q_FOREACH(const KimpanelLookupTable::Entry & entry, lookupTable.entries) {
        QVariantMap map;
        map[QStringLiteral("label")] = entry.label;
        map[QStringLiteral("text")] = entry.text;
        candidateList += map;
    }
    setData(QStringLiteral("LookupTable"), candidateList);
    setData(QStringLiteral("HasPrev"), lookupTable.has_prev);
    setData(QStringLiteral("HasNext"), lookupTable.has_next);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateSpotLocation(int x, int y)
{
    updateSpotRect(x, y, 0, 0);
}

void KimpanelInputPanelContainer::updateSpotRect(int x, int y, int w, int h)
{
    setData(QStringLiteral("Position"), QRect(x, y, w, h));
    checkForUpdate();
}

void KimpanelInputPanelContainer::showAux(bool visible)
{
    setData(QStringLiteral("AuxVisible"), visible);
    checkForUpdate();
}

void KimpanelInputPanelContainer::showPreedit(bool visible)
{
    setData(QStringLiteral("PreeditVisible"), visible);
    checkForUpdate();
}

void KimpanelInputPanelContainer::showLookupTable(bool visible)
{
    setData(QStringLiteral("LookupTableVisible"), visible);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateLookupTableCursor(int cursor)
{
    setData(QStringLiteral("LookupTableCursor"), cursor);
    checkForUpdate();
}

void KimpanelInputPanelContainer::updateLookupTableFull(const KimpanelLookupTable& lookupTable, int cursor, int layout)
{
    QVariantList candidateList;
    Q_FOREACH(const KimpanelLookupTable::Entry & entry, lookupTable.entries) {
        QVariantMap map;
        map[QStringLiteral("label")] = entry.label;
        map[QStringLiteral("text")] = entry.text;
        candidateList += map;
    }
    setData(QStringLiteral("LookupTable"), candidateList);
    setData(QStringLiteral("HasPrev"), lookupTable.has_prev);
    setData(QStringLiteral("HasNext"), lookupTable.has_next);
    setData(QStringLiteral("LookupTableCursor"), cursor);
    setData(QStringLiteral("LookupTableLayout"), layout);
    checkForUpdate();

}
