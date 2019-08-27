/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

#include "containmentinterface.h"

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

#include <QQuickItem>

#include <KActionCollection>

// FIXME HACK TODO: Unfortunately we have no choice but to hard-code a list of
// applets we know to expose the correct interface right now -- this is slated
// for replacement with some form of generic service.
QStringList ContainmentInterface::m_knownTaskManagers = QStringList() << QLatin1String("org.kde.plasma.taskmanager")
    << QLatin1String("org.kde.plasma.icontasks")
    << QLatin1String("org.kde.plasma.expandingiconstaskmanager");

ContainmentInterface::ContainmentInterface(QObject *parent) : QObject(parent)
{
}

ContainmentInterface::~ContainmentInterface()
{
}

bool ContainmentInterface::mayAddLauncher(QObject *appletInterface, ContainmentInterface::Target target, const QString &entryPath)
{
    if (!appletInterface) {
        return false;
    }

    Plasma::Applet *applet = appletInterface->property("_plasma_applet").value<Plasma::Applet *>();
    Plasma::Containment *containment = applet->containment();

    if (!containment) {
        return false;
    }

    Plasma::Corona *corona = containment->corona();

    if (!corona) {
        return false;
    }

    switch (target) {
        case Desktop: {
            containment = corona->containmentForScreen(containment->screen());

            if (containment) {
                return (containment->immutability() == Plasma::Types::Mutable);
            }

            break;
        }
        case Panel: {
            if (containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel"))
            {
                return (containment->immutability() == Plasma::Types::Mutable);
            }

            break;
        }
        case TaskManager: {
            if (!entryPath.isEmpty() && containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel"))
            {
                const Plasma::Applet *taskManager = nullptr;

                foreach(const Plasma::Applet *applet, containment->applets()) {
                    if (m_knownTaskManagers.contains(applet->pluginMetaData().pluginId())) {
                        taskManager = applet;

                        break;
                    }
                }

                if (taskManager) {
                    QQuickItem* gObj = qobject_cast<QQuickItem *>(taskManager->property("_plasma_graphicObject").value<QObject *>());

                    if (!gObj || !gObj->childItems().count()) {
                        return false;
                    }

                    QQuickItem *rootItem = gObj->childItems().first();

                    QVariant ret;

                    QMetaObject::invokeMethod(rootItem, "hasLauncher", Q_RETURN_ARG(QVariant, ret),
                        Q_ARG(QVariant, QUrl::fromLocalFile(entryPath)));

                    return !ret.toBool();
                }
            }

            break;
        }
    }

    return false;
}

void ContainmentInterface::addLauncher(QObject *appletInterface, ContainmentInterface::Target target, const QString &entryPath)
{
    if (!appletInterface) {
        return;
    }

    Plasma::Applet *applet = appletInterface->property("_plasma_applet").value<Plasma::Applet *>();
    Plasma::Containment *containment = applet->containment();

    if (!containment) {
        return;
    }

    Plasma::Corona *corona = containment->corona();

    if (!corona) {
        return;
    }

    switch (target) {
        case Desktop: {
            containment = corona->containmentForScreen(containment->screen());

            if (!containment) {
                return;
            }

            const QStringList &containmentProvides = KPluginMetaData::readStringList(containment->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

            if (containmentProvides.contains(QLatin1String("org.kde.plasma.filemanagement"))) {
                QQuickItem* gObj = qobject_cast<QQuickItem *>(containment->property("_plasma_graphicObject").value<QObject *>());

                if (!gObj || !gObj->childItems().count()) {
                    return;
                }

                QQuickItem *rootItem = nullptr;

                foreach(QQuickItem *item, gObj->childItems()) {
                    if (item->objectName() == QLatin1String("folder")) {
                        rootItem = item;

                        break;
                    }
                }

                if (rootItem) {
                    QMetaObject::invokeMethod(rootItem, "addLauncher", Q_ARG(QVariant, QUrl::fromLocalFile(entryPath)));
                }
            } else {
                containment->createApplet(QStringLiteral("org.kde.plasma.icon"), QVariantList() << entryPath);
            }

            break;
        }
        case Panel: {
            if (containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel"))
            {
                containment->createApplet(QStringLiteral("org.kde.plasma.icon"), QVariantList() << entryPath);
            }

            break;
        }
        case TaskManager: {
            if (containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel"))
            {
                const Plasma::Applet *taskManager = nullptr;

                foreach(const Plasma::Applet *applet, containment->applets()) {
                    if (m_knownTaskManagers.contains(applet->pluginMetaData().pluginId())) {
                        taskManager = applet;

                        break;
                    }
                }

                if (taskManager) {
                    QQuickItem* gObj = qobject_cast<QQuickItem *>(taskManager->property("_plasma_graphicObject").value<QObject *>());

                    if (!gObj || !gObj->childItems().count()) {
                        return;
                    }

                    QQuickItem *rootItem = gObj->childItems().first();

                    QMetaObject::invokeMethod(rootItem, "addLauncher", Q_ARG(QVariant, QUrl::fromLocalFile(entryPath)));
                }
            }

            break;
        }
    }
}

QObject* ContainmentInterface::screenContainment(QObject *appletInterface)
{
    if (!appletInterface) {
        return nullptr;
    }

    const Plasma::Applet *applet = appletInterface->property("_plasma_applet").value<Plasma::Applet *>();
    Plasma::Containment *containment = applet->containment();

    if (!containment) {
        return nullptr;
    }

    Plasma::Corona *corona = containment->corona();

    if (!corona) {
        return nullptr;
    }

    return corona->containmentForScreen(containment->screen());
}

bool ContainmentInterface::screenContainmentMutable(QObject *appletInterface)
{
    const Plasma::Containment *containment = static_cast<const Plasma::Containment *>(screenContainment(appletInterface));

    if (containment) {
        return (containment->immutability() == Plasma::Types::Mutable);
    }

    return false;
}

void ContainmentInterface::ensureMutable(Plasma::Containment *containment)
{
    if (containment && containment->immutability() != Plasma::Types::Mutable) {
         containment->actions()->action(QStringLiteral("lock widgets"))->trigger();
    }
}
