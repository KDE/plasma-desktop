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

#include "kimpanelstatusbarcontainer.h"
#include "kimpanelservice.h"
#include "kimpanelagent.h"
#include "kimpaneldataengine.h"
#include "input_sources.h"
#include "layout_list_models.h"

#include <QDebug>

KimpanelStatusBarContainer::KimpanelStatusBarContainer(QObject* parent, PanelAgent* panelAgent):
    DataContainer(parent),
    m_panelAgent(panelAgent)
{
    connect(m_panelAgent, SIGNAL(updateProperty(KimpanelProperty)), this, SLOT(updateProperty(KimpanelProperty)));
    connect(m_panelAgent, SIGNAL(registerProperties(QList<KimpanelProperty>)), this, SLOT(registerProperties(QList<KimpanelProperty>)));
    connect(m_panelAgent, SIGNAL(execMenu(QList<KimpanelProperty>)), this, SLOT(execMenu(QList<KimpanelProperty>)));
    connect(m_panelAgent, SIGNAL(execDialog(KimpanelProperty)), this, SLOT(execDialog(KimpanelProperty)));
    connect(m_panelAgent, &PanelAgent::showPlasmoid, this, &KimpanelStatusBarContainer::showPlasmoid);
    connect(m_panelAgent, &PanelAgent::currentLayoutChanged, this, &KimpanelStatusBarContainer::setCurrentLayout);
    setData("LayoutList", QVariant::fromValue(panelAgent->models()->currentLayoutListModel()));
    setData("LayoutModels", QVariant::fromValue(panelAgent->models()));
}

void KimpanelStatusBarContainer::showPlasmoid(bool visible)
{
    setData("Visibility", visible);
    checkForUpdate();
}

void KimpanelStatusBarContainer::setCurrentLayout(const QString &tag, const QString &iconName)
{
    qDebug() << tag << iconName;
    setData("CurrentLayoutIndex", m_panelAgent->models()->currentLayoutIndex());
    setData("CurrentTag", tag);
    setData("CurrentIconName", iconName);
    checkForUpdate();
}

Plasma::Service* KimpanelStatusBarContainer::service(QObject* parent)
{
    KimpanelService *controller = new KimpanelService(parent, QLatin1String(STATUSBAR_SOURCE_NAME), m_panelAgent);
    connect(this, SIGNAL(updateRequested(DataContainer*)),
            controller, SLOT(enableKimpanelOperations()));
    return controller;
}

void KimpanelStatusBarContainer::updateProperty(const KimpanelProperty& property)
{
    for (int i = 0; i < m_props.size(); i++) {
        if (m_props[i].key == property.key) {
            m_props[i] = property;
            QList<QVariant> varList;
            Q_FOREACH(const KimpanelProperty & prop, m_props) {
                if (prop.key == "/Fcitx/logo" || prop.key == "/Fcitx/im") {
                    continue;
                }
                varList << prop.toMap();
            }
            setData(QStringLiteral("Properties"), varList);
            checkForUpdate();
            break;
        }
    }
}

void KimpanelStatusBarContainer::registerProperties(const QList< KimpanelProperty >& props)
{
    m_props = props;
    QList<QVariant> varList;
    Q_FOREACH(const KimpanelProperty & prop, m_props) {
        if (prop.key == "/Fcitx/logo" || prop.key == "/Fcitx/im") {
            continue;
        }
        varList << prop.toMap();
    }
    setData(QStringLiteral("Properties"), varList);
    checkForUpdate();
}

void KimpanelStatusBarContainer::execMenu(const QList< KimpanelProperty >& props)
{
    QList<QVariant> varList;
    Q_FOREACH(const KimpanelProperty & prop, props) {
        varList << prop.toMap();
    }
    QVariantMap map;
    map[QStringLiteral("props")] = varList;
    map[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
    setData(QStringLiteral("Menu"), map);
    checkForUpdate();
}

void KimpanelStatusBarContainer::execDialog(const KimpanelProperty& prop)
{
    Q_UNUSED(prop)
}
