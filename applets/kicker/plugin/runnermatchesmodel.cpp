/***************************************************************************
 *   Copyright (C) 2012 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
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
#include "runnermodel.h"
#include "actionlist.h"

#include <QAction>
#include <QIcon>

#include <KLocalizedString>
#include <KRunner/RunnerManager>
#include <KRun>

#include <Plasma/Plasma>

RunnerMatchesModel::RunnerMatchesModel(const QString &runnerId, const QString &name,
    Plasma::RunnerManager *manager, QObject *parent)
: AbstractModel(parent)
, m_runnerId(runnerId)
, m_name(name)
, m_runnerManager(manager)
{
}

QString RunnerMatchesModel::description() const
{
    return m_name;
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
        if (!match.iconName().isEmpty()) {
            return match.iconName();
        }

        return match.icon();
    } else if (role == Kicker::DescriptionRole) {
        return match.subtext();
    } else if (role == Kicker::FavoriteIdRole) {
        if (match.runner()->id() == QLatin1String("services")) {
            return match.data().toString();
        }
    } else if (role == Kicker::UrlRole) {
        const QString &runnerId = match.runner()->id();
        if (runnerId == QLatin1String("baloosearch") || runnerId == QLatin1String("bookmarks")) {
            return QUrl(match.data().toString());
        } else if (runnerId == QLatin1String("recentdocuments")
                   || runnerId == QLatin1String("services")) {
            KService::Ptr service = KService::serviceByStorageId(match.data().toString());
            if (service) {
                return QUrl::fromLocalFile(Kicker::resolvedServiceEntryPath(service));
            }
        }
    } else if (role == Kicker::HasActionListRole) {
        // Hack to expose the protected Plasma::AbstractRunner::actions() method.
        class MyRunner : public Plasma::AbstractRunner
        {
            public:
                using Plasma::AbstractRunner::actions;
        };

        MyRunner *runner = static_cast<MyRunner *>(match.runner());

        Q_ASSERT(runner);

        return match.runner()->id() == QLatin1String("services") || !runner->actions().isEmpty();
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList;

        foreach (QAction *action, m_runnerManager->actionsForMatch(match)) {
            QVariantMap item = Kicker::createActionItem(action->text(), QStringLiteral("runnerAction"),
                QVariant::fromValue<QObject *>(action));
            item[QStringLiteral("icon")] = action->icon();

            actionList << item;
        }

        // Only try to get a KService for matches from the services runner. Assuming
        // that any other runner returns something we want to turn into a KService is
        // unsafe, e.g. files from the Baloo runner might match a storageId just by
        // accident, creating a dangerous false positive.
        if (match.runner()->id() != QLatin1String("services")) {
            return actionList;
        }

        const KService::Ptr service = KService::serviceByStorageId(match.data().toString());

        if (service) {
            if (!actionList.isEmpty()) {
                actionList << Kicker::createSeparatorActionItem();
            }

            const QVariantList &jumpListActions = Kicker::jumpListActions(service);
            if (!jumpListActions.isEmpty()) {
                actionList << jumpListActions << Kicker::createSeparatorActionItem();
            }

            QObject *appletInterface = static_cast<RunnerModel *>(parent())->appletInterface();

            const bool systemImmutable = appletInterface->property("immutability").toInt() == Plasma::Types::SystemImmutable;

            const QVariantList &addLauncherActions = Kicker::createAddLauncherActionList(appletInterface, service);
            if (!systemImmutable && !addLauncherActions.isEmpty()) {
                actionList << addLauncherActions << Kicker::createSeparatorActionItem();
            }

            const QVariantList &recentDocuments = Kicker::recentDocumentActions(service);
            if (!recentDocuments.isEmpty()) {
                actionList << recentDocuments << Kicker::createSeparatorActionItem();
            }

            // Don't allow adding launchers, editing, hiding, or uninstalling applications
            // when system is immutable.
            if (systemImmutable) {
                return actionList;
            }

            if (service->isApplication()) {
                actionList << Kicker::editApplicationAction(service);
                actionList << Kicker::appstreamActions(service);
            }
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

    QObject *appletInterface = static_cast<RunnerModel *>(parent())->appletInterface();

    const KService::Ptr service = KService::serviceByStorageId(match.data().toString());

    if (Kicker::handleAddLauncherAction(actionId, appletInterface, service)) {
        return true;
    } else if (Kicker::handleEditApplicationAction(actionId, service)) {
        return true;
    } else if (Kicker::handleAppstreamActions(actionId, argument)) {
        return true;
    } else if (actionId == QLatin1String("_kicker_jumpListAction")) {
        return KRun::run(argument.toString(), {}, nullptr, service ? service->name() : QString(), service ? service->icon() : QString());
    } else if (actionId == QLatin1String("_kicker_recentDocument")
        || actionId == QLatin1String("_kicker_forgetRecentDocuments")) {
        return Kicker::handleRecentDocumentAction(service, actionId, argument);
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
            m_matches[row] = matches.at(row);
        }
    }

    if (emitDataChange) {
        emit dataChanged(index(0, 0), index(ceiling - 1, 0));
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

    if (emitCountChange) {
        emit countChanged();
    }
}

AbstractModel *RunnerMatchesModel::favoritesModel()
{
    return static_cast<RunnerModel *>(parent())->favoritesModel();
}
