/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef TOUCHPADSERVICE_H
#define TOUCHPADSERVICE_H

#include <Plasma/Service>

class OrgKdeTouchpadInterface;

class TouchpadService : public Plasma::Service
{
    Q_OBJECT
public:
    TouchpadService(OrgKdeTouchpadInterface *daemon, const QString &destination, QObject *parent = nullptr);
    ~TouchpadService();

protected:
    Plasma::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters) override;

private:
    OrgKdeTouchpadInterface *m_daemon;
    QString m_destination;
};

#endif // TOUCHPADSERVICE_H
