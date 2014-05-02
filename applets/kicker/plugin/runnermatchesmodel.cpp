/***************************************************************************
 *   Copyright (C) 2012 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "runnermatchesmodel.h"
#include "actionlist.h"

#include <QAction>
#include <QIcon>

#include <KRunner/RunnerManager>

RunnerMatchesModel::RunnerMatchesModel(const QString &runnerId, const QString &name,
    Plasma::RunnerManager *manager, QObject *parent)
: AbstractModel(parent)
, m_runnerId(runnerId)
, m_name(name)
, m_runnerManager(manager)
{
}

QVariant RunnerMatchesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_matches.count()) {
        return QVariant();
    }

    Plasma::QueryMatch match = m_matches.at(index.row());

    if (role == Qt::DisplayRole) {
        return match.text();
    } else if (role == Qt::DecorationRole) {
        return match.icon();
    } else if (role == Kicker::FavoriteIdRole) {
        if (m_runnerId == "services") {
            return QVariant("app:" + match.data().toString());
        }
    } else if (role == Kicker::HasActionListRole) {
        // Hack to expose the protected Plasma::AbstractRunner::actions() method.
        class MyRunner : public Plasma::AbstractRunner
        {
            public:
                using Plasma::AbstractRunner::actions;;
        };

        MyRunner *runner = static_cast<MyRunner *>(match.runner());

        Q_ASSERT(runner);

        return !runner->actions().isEmpty();
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList;

        foreach (QAction *action, m_runnerManager->actionsForMatch(match)) {
            QVariantMap item = Kicker::createActionItem(action->text(), "runnerAction",
                QVariant::fromValue<QObject *>(action));
            item["icon"] = action->icon();

            actionList << item;
        }

        return actionList;
    }

    return QVariant();
}

int RunnerMatchesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_matches.count();
}

bool RunnerMatchesModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= m_matches.count()) {
        return false;
    }

    Plasma::QueryMatch match = m_matches.at(row);

    if (!match.isEnabled()) {
        return false;
    }

    if (!actionId.isEmpty()) {
        QObject *obj = argument.value<QObject *>();

        if (!obj) {
            return false;
        }

        QAction *action = qobject_cast<QAction *>(obj);

        if (!action) {
            return false;
        }

        match.setSelectedAction(action);
    } else if (m_runnerId == "services") {
        emit appLaunched(match.data().toString());
    }

    m_runnerManager->run(match);

    return true;
}

void RunnerMatchesModel::setMatches(const QList< Plasma::QueryMatch > &matches)
{
    int oldCount = m_matches.count();
    int newCount = matches.count();

    bool emitCountChange = (oldCount != newCount);

    int ceiling = qMin(oldCount, newCount);
    bool emitDataChange = false;

    for (int row = 0; row < ceiling; ++row) {
        if (!(m_matches.at(row) == matches.at(row))) {
            emitDataChange = true;

            break;
        }
    }

    if (newCount > oldCount) {
        beginInsertRows(QModelIndex(), oldCount, newCount - 1);

        m_matches = matches;

        endInsertRows();
    } else if (newCount < oldCount) {
        beginRemoveRows(QModelIndex(), newCount, oldCount - 1);

        m_matches = matches;

        endRemoveRows();
    }

    if (emitDataChange) {
        m_matches = matches;

        emit dataChanged(index(0, 0), index(ceiling - 1, 0));
    }

    if (emitCountChange) {
        emit countChanged();
    }
}
