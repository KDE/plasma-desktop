/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <Plasma5Support/DataEngine>

class OrgKdeTouchpadInterface;

class TouchpadEngine : public Plasma5Support::DataEngine
{
    Q_OBJECT

public:
    TouchpadEngine(QObject *parent);
    ~TouchpadEngine();

    Plasma5Support::Service *serviceForSource(const QString &source) override;

private Q_SLOTS:
    void workingTouchpadFoundChanged(bool);
    void mousePluggedInChanged(bool);
    void enabledChanged(bool);

private:
    void init();
    QString m_source;
    OrgKdeTouchpadInterface *m_daemon;
};
