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

#include "kimpaneljob.h"
#include "kimpanelagent.h"

KimpanelJob::KimpanelJob(PanelAgent* panelAgent,
                         const QString& destination,
                         const QString& operation,
                         const QMap< QString, QVariant >& parameters, QObject* parent):
    Plasma::ServiceJob(destination, operation, parameters, parent),
    m_panelAgent(panelAgent)
{

}

void KimpanelJob::start()
{
    const QString operation(operationName());
    if (operation == QLatin1String("LookupTablePageUp")) {
        m_panelAgent->lookupTablePageUp();
    } else if (operation == QLatin1String("LookupTablePageDown")) {
        m_panelAgent->lookupTablePageDown();
    } else if (operation == QLatin1String("MovePreeditCaret")) {
        if (parameters().contains(QLatin1String("position"))) {
            int position = parameters()[QStringLiteral("position")].toInt();
            m_panelAgent->movePreeditCaret(position);
        }
    } else if (operation == QLatin1String("SelectCandidate")) {
        if (parameters().contains(QLatin1String("candidate"))) {
            int candidate = parameters()[QStringLiteral("candidate")].toInt();
            m_panelAgent->selectCandidate(candidate);
        }
    } else if (operation == QLatin1String("TriggerProperty")) {
        if (parameters().contains(QLatin1String("key"))) {
            QString key = parameters()[QStringLiteral("key")].toString();
            m_panelAgent->triggerProperty(key);
        }
    } else if (operation == QLatin1String("ReloadConfig")) {
        m_panelAgent->reloadConfig();
    } else if (operation == QLatin1String("Configure")) {
        m_panelAgent->configure();
    } else if (operation == QLatin1String("Exit")) {
        m_panelAgent->exit();
    }
}
