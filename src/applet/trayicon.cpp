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

#include <Plasma/ToolTipManager>

K_EXPORT_PLASMA_APPLET(touchpad, TrayIcon)

TrayIcon::TrayIcon(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_interface(0), m_enabled(false)
{
    m_toggleAction = new KAction(i18n("Enable touchpad"), this);
    m_toggleAction->setCheckable(true);

    m_touchpadIcon = new Plasma::Svg(this);
    m_touchpadIcon->setImagePath("icons/touchpad");
    m_touchpadIcon->setContainsMultipleImages(true);

    resize(48, 48);
}

QList<QAction*> TrayIcon::contextualActions()
{
    return QList<QAction*>() << m_toggleAction;
}

void TrayIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_interface) {
        m_interface->safeToggle();
    }
    Applet::mousePressEvent(event);
}

void TrayIcon::init()
{
    Applet::init();

    QDBusInterface kded(QLatin1String("org.kde.kded"), QLatin1String("/kded"));
    kded.call("loadModule", "touchpad");

    m_interface = new OrgKdeTouchpadInterface("org.kde.kded",
                                              "/modules/touchpad",
                                              QDBusConnection::sessionBus(),
                                              this);

    if (!m_interface->isValid()) {
        setFailedToLaunch(true, i18n("Can't connect to daemon"));
        return;
    }

    if (!m_interface->workingTouchpadFound()) {
        setFailedToLaunch(true, i18n("Touchpad not found"));
        return;
    }

    setState(m_interface->isEnabled());
    connect(m_interface, SIGNAL(enabledChanged(bool)), SLOT(setState(bool)));
    connect(this, SIGNAL(activate()), m_interface, SLOT(toggle()));
    connect(m_toggleAction, SIGNAL(triggered()),
            m_interface, SLOT(safeToggle()));
}

void TrayIcon::paintInterface(QPainter *painter,
                              const QStyleOptionGraphicsItem *,
                              const QRect &contentsRect)
{
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    m_touchpadIcon->resize(QSizeF(contentsRect.size()));
    m_touchpadIcon->paint(painter, QRectF(contentsRect),
                          m_enabled ? "touchpad_enabled" : "touchpad_disabled");
    painter->restore();
}

void TrayIcon::setState(bool enabled)
{
    m_enabled = enabled;

    Plasma::ToolTipContent tip;
    tip.setMainText(i18n("Touchpad"));
    tip.setSubText(m_enabled ? i18n("Touchpad is enabled") :
                               i18n("Touchpad is disabled"));
    m_touchpadIcon->resize(48, 48);
    tip.setImage(m_touchpadIcon->pixmap(
                     m_enabled ? "touchpad_enabled" : "touchpad_disabled"));
    setToolTip(tip.subText());
    Plasma::ToolTipManager::self()->setContent(this, tip);

    m_toggleAction->setChecked(m_enabled);

    update();

    //Short delay before marking applet as inactive.
    //Isn't required, but looks nice
    QTimer::singleShot(m_enabled ? 1000 : 0, this, SLOT(updateStatus()));
}

void TrayIcon::updateStatus()
{
    setStatus(m_enabled ? Plasma::PassiveStatus : Plasma::ActiveStatus);
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
