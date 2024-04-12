/*
    SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

// Qt
#include <QSortFilterProxyModel>

// KDE
#include <PlasmaActivities/ActivitiesModel>
#include <PlasmaActivities/Consumer>

#include <config-X11.h> // Required by files that include this header

namespace TaskManager
{
class WindowTasksModel;
}

class SortedActivitiesModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool inhibitUpdates READ inhibitUpdates WRITE setInhibitUpdates NOTIFY inhibitUpdatesChanged)

public:
    SortedActivitiesModel(const QList<KActivities::Info::State> &states, QObject *parent = nullptr);
    ~SortedActivitiesModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    QString relativeActivity(int relative) const;

    TaskManager::WindowTasksModel *const m_windowTasksModel;

protected:
    uint lastUsedTime(const QString &activity) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    enum AdditionalRoles {
        LastTimeUsed = KActivities::ActivitiesModel::UserRole,
        LastTimeUsedString = KActivities::ActivitiesModel::UserRole + 1,
        WindowCount = KActivities::ActivitiesModel::UserRole + 2,
        HasWindows = KActivities::ActivitiesModel::UserRole + 3,
    };

public Q_SLOTS:
    bool inhibitUpdates() const;
    void setInhibitUpdates(bool sortByLastUsedTime);

    void onBackgroundsUpdated(const QStringList &changedBackgrounds);
    void onCurrentActivityChanged(const QString &currentActivity);

    QString activityIdForRow(int row) const;
    QString activityIdForIndex(const QModelIndex &index) const;
    int rowForActivityId(const QString &activity) const;

    void rowChanged(int row, const QList<int> &roles);

    void onWindowAdded(const QModelIndex &parent, int first, int last);
    void onWindowRemoved(const QModelIndex &parent, int first, int last);
    void onWindowChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);

Q_SIGNALS:
    void inhibitUpdatesChanged(bool inhibitUpdates);

private:
    bool m_inhibitUpdates;

    QString m_previousActivity;

    KActivities::ActivitiesModel *m_activitiesModel = nullptr;
    KActivities::Consumer *m_activities = nullptr;

    QHash<QString, QVariantList> m_activitiesWindows;

    QVariant getWinIdList(const QModelIndex &parent, int row);
};
