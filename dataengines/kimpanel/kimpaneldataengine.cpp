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

#include "kimpaneldataengine.h"
#include "kimpanelagent.h"
#include "kimpanelservice.h"
#include "kimpanelinputpanelcontainer.h"
#include "kimpanelstatusbarcontainer.h"

// Plasma
#include <Plasma/DataContainer>

KimpanelEngine::KimpanelEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args), m_panelAgent(0)
{
}

void KimpanelEngine::init()
{
    m_panelAgent = new PanelAgent(this);
    KimpanelInputPanelContainer* inputpanelSource = new KimpanelInputPanelContainer(this, m_panelAgent);
    inputpanelSource->setObjectName(INPUTPANEL_SOURCE_NAME);
    KimpanelStatusBarContainer* statusbarSource = new KimpanelStatusBarContainer(this, m_panelAgent);
    statusbarSource->setObjectName(STATUSBAR_SOURCE_NAME);
    addSource(inputpanelSource);
    addSource(statusbarSource);
    this->m_panelAgent->created();
}

Plasma::Service* KimpanelEngine::serviceForSource(const QString& source)
{
    if (source == INPUTPANEL_SOURCE_NAME) {
        KimpanelInputPanelContainer* container = qobject_cast< KimpanelInputPanelContainer* >(containerForSource(source));
        if (container)
            return container->service(this);
    } else if (source == STATUSBAR_SOURCE_NAME) {
        KimpanelStatusBarContainer* container = qobject_cast< KimpanelStatusBarContainer* >(containerForSource(source));
        if (container)
            return container->service(this);
    }

    return Plasma::DataEngine::serviceForSource(source);
}

K_EXPORT_PLASMA_DATAENGINE(kimpanel, KimpanelEngine)
