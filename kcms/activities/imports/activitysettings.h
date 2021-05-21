/*
    SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ACTIVITYSETTINGS_H
#define ACTIVITYSETTINGS_H

#include <QObject>

class ActivitySettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool newActivityAuthorized READ newActivityAuthorized CONSTANT)

public:
    explicit ActivitySettings(QObject *parent = nullptr);
    ~ActivitySettings() override;

    bool newActivityAuthorized() const;

public Q_SLOTS:
    Q_INVOKABLE void configureActivities();
    Q_INVOKABLE void configureActivity(const QString &id);
    Q_INVOKABLE void newActivity();
    Q_INVOKABLE void deleteActivity(const QString &id);

private:
    bool m_newActivityAuthorized;
};

#endif // ACTIVITYSETTINGS_H
