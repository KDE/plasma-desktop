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

#include "kimpanelservice.h"
#include "kimpaneljob.h"
#include "kimpaneldataengine.h"

KimpanelService::KimpanelService(QObject* parent, const QString& name, PanelAgent* panelAgent):
    Service(parent), m_panelAgent(panelAgent)
{
    setName("kimpanel");
    setObjectName(name);
    setDestination(name);
    enableKimpanelOperations();
}

void KimpanelService::enableKimpanelOperations()
{
    if (destination() == INPUTPANEL_SOURCE_NAME) {
        setOperationEnabled("LookupTablePageUp", true);
        setOperationEnabled("LookupTablePageDown", true);
        setOperationEnabled("MovePreeditCaret", true);
        setOperationEnabled("SelectCandidate", true);
    } else if (destination() == STATUSBAR_SOURCE_NAME) {
        setOperationEnabled("TriggerProperty", true);
        setOperationEnabled("Exit", true);
        setOperationEnabled("ReloadConfig", true);
        setOperationEnabled("Configure", true);
    }
}

Plasma::ServiceJob* KimpanelService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    return new KimpanelJob(m_panelAgent, destination(), operation, parameters, this);
}
