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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <KIcon>
#include <KAction>

#include <Plasma/Applet>

#include "touchpadinterface.h"

class TrayIcon : public Plasma::Applet
{
    Q_OBJECT
public:
    TrayIcon(QObject *, const QVariantList &);

    void init();
    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);
    QList<QAction*> contextualActions();

protected:
    void createConfigurationInterface(KConfigDialog *parent);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private Q_SLOTS:
    void setState(bool);
    void updateStatus();

private:
    OrgKdeTouchpadInterface *m_interface;
    KIcon *m_touchpadIcon;
    KAction *m_toggleAction;
    bool m_enabled;
};

#endif // TRAYICON_H
