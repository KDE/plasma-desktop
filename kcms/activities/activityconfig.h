/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <PlasmaActivities/Controller>

#include <QKeySequence>
#include <QtQml/QQmlModuleRegistration>

class ActivityConfig : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString activityId READ activityId WRITE setActivityId NOTIFY activityIdChanged)
    Q_PROPERTY(QString name MEMBER m_name NOTIFY infoChanged)
    Q_PROPERTY(QString description MEMBER m_description NOTIFY infoChanged)
    Q_PROPERTY(QString iconName MEMBER m_iconName NOTIFY infoChanged)
    Q_PROPERTY(bool isPrivate MEMBER m_private NOTIFY infoChanged)
    Q_PROPERTY(QKeySequence shortcut MEMBER m_shortcut NOTIFY infoChanged)
    Q_PROPERTY(bool isSaveNeeded READ isSaveNeeded NOTIFY infoChanged)
    Q_PROPERTY(bool inhibitScreen MEMBER m_inhibitScreen NOTIFY infoChanged)
    Q_PROPERTY(bool inhibitSleep MEMBER m_inhibitSleep NOTIFY infoChanged)

public:
    explicit ActivityConfig(QObject *parent = nullptr);
    ~ActivityConfig();

public:
    QString activityId() const;
    void setActivityId(const QString &activityId);

    bool isSaveNeeded();

    Q_INVOKABLE void save();

private:
    void load();
    void reset();
    void createActivity();

Q_SIGNALS:
    void activityIdChanged();
    void infoChanged();

private:
    QString m_activityId;

    QString m_name;
    QString m_description;
    QString m_iconName;
    bool m_private;
    QKeySequence m_shortcut;
    bool m_inhibitScreen = false;
    bool m_savedInhibitScreen = false;
    bool m_inhibitSleep = false;
    bool m_savedInhibitSleep = false;

    KActivities::Controller m_activities;
    bool m_savedPrivate = false;
};
