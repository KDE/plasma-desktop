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
#ifndef TOUCHPADENGINE_H
#define TOUCHPADENGINE_H

#include <Plasma/DataEngine>

class OrgKdeTouchpadInterface;

class TouchpadEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    TouchpadEngine(QObject *parent, const QVariantList &args);
    ~TouchpadEngine();

    Plasma::Service *serviceForSource(const QString &source) override;

private Q_SLOTS:
    void workingTouchpadFoundChanged(bool);
    void mousePluggedInChanged(bool);
    void enabledChanged(bool);

private:
    void init();
    QString m_source;
    OrgKdeTouchpadInterface *m_daemon;
};

#endif // TOUCHPADENGINE_H
