/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
