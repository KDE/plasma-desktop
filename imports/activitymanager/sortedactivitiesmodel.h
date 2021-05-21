/*
    SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SORTED_ACTIVITIES_MODEL_H
#define SORTED_ACTIVITIES_MODEL_H

// Qt
#include <QSortFilterProxyModel>
#include <QWidgetList> //For WId

// KDE
#include <KActivities/ActivitiesModel>
#include <KActivities/Consumer>
#include <KActivities/Info>

#include <config-X11.h>

#include <netwm.h>

class SortedActivitiesModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool inhibitUpdates READ inhibitUpdates WRITE setInhibitUpdates NOTIFY inhibitUpdatesChanged)

public:
    SortedActivitiesModel(const QVector<KActivities::Info::State> &states, QObject *parent = nullptr);
    ~SortedActivitiesModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    QString relativeActivity(int relative) const;

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

    void rowChanged(int row, const QVector<int> &roles);

    void onWindowAdded(WId window);
    void onWindowRemoved(WId window);
    void onWindowChanged(WId window, NET::Properties properties, NET::Properties2 properties2);

Q_SIGNALS:
    void inhibitUpdatesChanged(bool inhibitUpdates);

private:
    bool m_inhibitUpdates;

    QString m_previousActivity;

    KActivities::ActivitiesModel *m_activitiesModel = nullptr;
    KActivities::Consumer *m_activities = nullptr;

    QHash<QString, QVector<WId>> m_activitiesWindows;
};

#endif // SORTED_ACTIVITIES_MODEL_H
