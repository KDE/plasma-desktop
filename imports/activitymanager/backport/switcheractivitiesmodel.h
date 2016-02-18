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

#ifndef ACTIVITIES_ACTIVITIESMODEL_H
#define ACTIVITIES_ACTIVITIESMODEL_H

// Qt
#include <QObject>
#include <QAbstractListModel>

// STL
#include <memory>

#include <KActivities/Info>

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivitiesBackport {

using namespace KActivities;

class ActivitiesModelPrivate;

/**
 * Data model that shows existing activities
 */
class KACTIVITIES_EXPORT ActivitiesModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(QVector<Info::State> shownStates READ shownStates WRITE setShownStates NOTIFY shownStatesChanged)

public:
    ActivitiesModel(QObject *parent = 0);

    /**
     * Constructs the model and sets the shownStates
     */
    ActivitiesModel(QVector<Info::State> shownStates, QObject *parent = 0);
    virtual ~ActivitiesModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const
        Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
        Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    enum Roles {
        ActivityId          = Qt::UserRole,       ///< UUID of the activity
        ActivityName        = Qt::UserRole + 1,   ///< Activity name
        ActivityDescription = Qt::UserRole + 2,   ///< Activity description
        ActivityIconSource  = Qt::UserRole + 3,   ///< Activity icon source name
        ActivityState       = Qt::UserRole + 4,   ///< The current state of the activity @see Info::State
        ActivityBackground  = Qt::UserRole + 5,   ///< Activity wallpaper (currently unsupported)
        ActivityIsCurrent   = Qt::UserRole + 6,   ///< Is this activity the current one current

        UserRole            = Qt::UserRole + 32   ///< To be used by models that inherit this one
    };

public Q_SLOTS:
    /**
     * The model can filter the list of activities based on their state.
     * This method sets which states should be shown.
     */
    void setShownStates(const QVector<Info::State> &shownStates);

    /**
     * The model can filter the list of activities based on their state.
     * This method returns which states are currently shown.
     */
    QVector<Info::State> shownStates() const;

Q_SIGNALS:
    void shownStatesChanged(const QVector<Info::State> &state);

private:
    friend class ActivitiesModelPrivate;
    ActivitiesModelPrivate * const d;
};

} // namespace KActivitiesBackport

#endif // ACTIVITIES_ACTIVITIESMODEL_H

