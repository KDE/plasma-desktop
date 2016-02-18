/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ACTIVITIES_ACTIVITYMODEL_P_H
#define ACTIVITIES_ACTIVITYMODEL_P_H

#include <QObject>
#include <KActivities/Consumer>

#include "model_updaters.h"
#include "switcheractivitiesmodel.h"
#include "qflatset.h"

namespace KActivitiesBackport {

class ActivitiesModelPrivate : public QObject {
    Q_OBJECT
public:
    ActivitiesModelPrivate(ActivitiesModel *parent);

public Q_SLOTS:
    void onActivityNameChanged(const QString &name);
    void onActivityDescriptionChanged(const QString &description);
    void onActivityIconChanged(const QString &icon);
    void onActivityStateChanged(KActivities::Info::State state);

    void replaceActivities(const QStringList &activities);
    void onActivityAdded(const QString &id, bool notifyClients = true);
    void onActivityRemoved(const QString &id);
    void onCurrentActivityChanged(const QString &id);

    void setServiceStatus(KActivities::Consumer::ServiceStatus status);

public:
    KActivities::Consumer activities;
    QVector<Info::State> shownStates;

    typedef std::shared_ptr<Info> InfoPtr;

    struct InfoPtrComparator {
        bool operator() (const InfoPtr& left, const InfoPtr& right) const
        {
            const QString &leftName = left->name().toLower();
            const QString &rightName = right->name().toLower();

            return
                (leftName < rightName) ||
                (leftName == rightName && left->id() < right->id());
        }
    };

    QFlatSet<InfoPtr, InfoPtrComparator> knownActivities;
    QFlatSet<InfoPtr, InfoPtrComparator> shownActivities;

    InfoPtr registerActivity(const QString &id);
    void unregisterActivity(const QString &id);
    void showActivity(InfoPtr activityInfo, bool notifyClients);
    void hideActivity(const QString &id);
    void backgroundsUpdated(const QStringList &activities);

    InfoPtr findActivity(QObject *ptr) const;

    ActivitiesModel *const q;

    DECLARE_RAII_MODEL_UPDATERS(ActivitiesModel)
};

} // namespace KActivitiesBackport

#endif // ACTIVITIES_ACTIVITYMODEL_P_H

