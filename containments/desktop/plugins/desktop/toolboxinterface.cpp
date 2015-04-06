/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                   *
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

#include "toolboxinterface.h"

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

#include <QEvent>
#include <QQuickItem>

ToolboxInterface::ToolboxInterface(QObject *parent) : QObject(parent)
, m_showToolbox(true)
, m_toolboxFound(false)
, m_appletInterface(0)
{
}

ToolboxInterface::~ToolboxInterface()
{
}

bool ToolboxInterface::showToolbox() const
{
    return m_showToolbox;
}

void ToolboxInterface::setShowToolbox(bool show)
{
    if (m_showToolbox != show) {
        m_showToolbox = show;

        setToolboxVisible(show);

        emit showToolboxChanged();
    }
}

bool ToolboxInterface::toolboxFound() const
{
    return m_toolboxFound;
}

QObject* ToolboxInterface::appletInterface() const
{
    return m_appletInterface;
}

void ToolboxInterface::setAppletInterface(QObject* appletInterface)
{
    if (m_appletInterface != appletInterface) {
        m_appletInterface = appletInterface;

        setToolboxVisible(m_showToolbox);

        emit appletInterfaceChanged();
    }
}

bool ToolboxInterface::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event->type() == QEvent::ChildAdded) {
        setToolboxVisible(m_showToolbox);
    }

    return false;
}

void ToolboxInterface::setToolboxVisible(bool visible)
{
    if (!m_appletInterface) {
        return;
    }

    Plasma::Applet *applet = m_appletInterface->property("_plasma_applet").value<Plasma::Applet *>();
    Plasma::Containment *containment = applet->containment();

    if (!containment) {
        return;
    }

    Plasma::Corona *corona = containment->corona();

    if (!corona) {
        return;
    }

    containment = corona->containmentForScreen(containment->screen());

    if (!containment) {
        return;
    }

    QQuickItem* gObj = qobject_cast<QQuickItem *>(containment->property("_plasma_graphicObject").value<QObject *>());

    if (!gObj || !gObj->childItems().count()) {
        return;
    }

    gObj->installEventFilter(this);

    foreach(QQuickItem *item, gObj->childItems()) {
        if (item->objectName() == QStringLiteral("org.kde.desktoptoolbox")) {
            m_toolboxFound = true;
            emit toolboxFoundChanged();

            item->setVisible(visible);

            return;
        }
    }
}
