/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "trayicon.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <KConfigDialog>
#include <KCModuleProxy>
#include <KCModuleInfo>
#include <KLocale>

K_EXPORT_PLASMA_APPLET(touchpad, TrayIcon)

TrayIcon::TrayIcon(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_enabled(false)
{
    m_interface = new OrgKdeTouchpadInterface("org.kde.kded",
                                              "/modules/touchpad",
                                              QDBusConnection::sessionBus(),
                                              this);

    m_touchpadIcon = new KIcon("input-touchpad");
    m_toggleAction = new KAction(i18n("Enable touchpad"), this);
    m_toggleAction->setCheckable(true);

    resize(48, 48);
}

QList<QAction*> TrayIcon::contextualActions()
{
    return QList<QAction*>() << m_toggleAction;
}

void TrayIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT activate();
    }
    Applet::mousePressEvent(event);
}

void TrayIcon::init()
{
    Applet::init();

    if (!m_interface->isValid()) {
        setFailedToLaunch(true, i18n("Can't connect to daemon"));
        return;
    }

    setState(m_interface->isEnabled());
    connect(m_interface, SIGNAL(enabledChanged(bool)), SLOT(setState(bool)));
    connect(this, SIGNAL(activate()), m_interface, SLOT(toggle()));
    connect(m_toggleAction, SIGNAL(triggered()), m_interface, SLOT(toggle()));
}

void TrayIcon::paintInterface(QPainter *painter,
                              const QStyleOptionGraphicsItem *,
                              const QRect &contentsRect)
{
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing);
    QPixmap pixmap(m_touchpadIcon->pixmap(contentsRect.size(),
                                          m_enabled ? QIcon::Normal :
                                                      QIcon::Disabled));
    painter->drawPixmap(contentsRect, pixmap);
    painter->restore();
}

void TrayIcon::setState(bool enabled)
{
    m_enabled = enabled;
    setStatus(m_enabled ? Plasma::PassiveStatus : Plasma::ActiveStatus);
    setToolTip(m_enabled ? i18n("Touchpad is enabled") :
                           i18n("Touchpad is disabled"));
    m_toggleAction->setChecked(m_enabled);
    update();
}

void TrayIcon::createConfigurationInterface(KConfigDialog *parent)
{
    Applet::createConfigurationInterface(parent);

    KCModuleProxy *proxy = new KCModuleProxy("kcm_touchpad", parent);
    proxy->load();

    connect(proxy, SIGNAL(changed(bool)), parent, SLOT(settingsModified(bool)));
    connect(parent, SIGNAL(okClicked()), proxy->realModule(), SLOT(save()));
    connect(parent, SIGNAL(applyClicked()), proxy->realModule(), SLOT(save()));

    parent->addPage(proxy, proxy->moduleInfo().moduleName(),
                    proxy->moduleInfo().icon(), proxy->moduleInfo().comment(),
                    false);

}
