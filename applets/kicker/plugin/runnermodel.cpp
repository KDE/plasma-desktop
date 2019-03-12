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

#include "runnermodel.h"
#include "runnermatchesmodel.h"

#include <QSet>

#include <KLocalizedString>
#include <KRunner/AbstractRunner>
#include <KRunner/RunnerManager>

RunnerModel::RunnerModel(QObject *parent) : QAbstractListModel(parent)
, m_favoritesModel(nullptr)
, m_appletInterface(nullptr)
, m_runnerManager(nullptr)
, m_mergeResults(false)
, m_deleteWhenEmpty(false)
{
    m_queryTimer.setSingleShot(true);
    m_queryTimer.setInterval(10);
    connect(&m_queryTimer, SIGNAL(timeout()), this, SLOT(startQuery()));
}

RunnerModel::~RunnerModel()
{
}

QHash<int, QByteArray> RunnerModel::roleNames() const
{
    return {{ Qt::DisplayRole, "display" }};
}

AbstractModel *RunnerModel::favoritesModel() const
{
    return m_favoritesModel;
}

void RunnerModel::setFavoritesModel(AbstractModel *model)
{
    if (m_favoritesModel != model) {
        m_favoritesModel = model;

        clear();

        if (!m_query.isEmpty()) {
            m_queryTimer.start();
        }

        emit favoritesModelChanged();
    }
}

QObject *RunnerModel::appletInterface() const
{
    return m_appletInterface;
}

void RunnerModel::setAppletInterface(QObject *appletInterface)
{
    if (m_appletInterface != appletInterface) {
        m_appletInterface = appletInterface;

        clear();

        if (!m_query.isEmpty()) {
            m_queryTimer.start();
        }

        emit appletInterfaceChanged();
    }
}

bool RunnerModel::deleteWhenEmpty() const
{
    return m_deleteWhenEmpty;
}

void RunnerModel::setDeleteWhenEmpty(bool deleteWhenEmpty)
{
    if (m_deleteWhenEmpty != deleteWhenEmpty) {
        m_deleteWhenEmpty = deleteWhenEmpty;

        clear();

        if (!m_query.isEmpty()) {
            m_queryTimer.start();
        }

        emit deleteWhenEmptyChanged();
    }
}

bool RunnerModel::mergeResults() const
{
    return m_mergeResults;
}

void RunnerModel::setMergeResults(bool merge)
{
    if (m_mergeResults != merge) {
        m_mergeResults = merge;

        clear();

        if (!m_query.isEmpty()) {
            m_queryTimer.start();
        }

        emit mergeResultsChanged();
    }
}

QVariant RunnerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_models.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_models.at(index.row())->name();
    }

    return QVariant();
}

int RunnerModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_models.count();
}

int RunnerModel::count() const
{
    return rowCount();
}

QObject *RunnerModel::modelForRow(int row)
{
    if (row < 0 || row >= m_models.count()) {
        return nullptr;
    }

    return m_models.at(row);
}

QStringList RunnerModel::runners() const
{
    return m_runners;
}

void RunnerModel::setRunners(const QStringList &runners)
{
    if (m_runners.toSet() != runners.toSet()) {
        m_runners = runners;

        if (m_runnerManager) {
            m_runnerManager->setAllowedRunners(runners);
        }

        emit runnersChanged();
    }
}

QString RunnerModel::query() const
{
    return m_query;
}

void RunnerModel::setQuery(const QString &query)
{
    if (m_query != query) {
        m_query = query;

        m_queryTimer.start();

        emit queryChanged();
    }
}

void RunnerModel::startQuery()
{
    if (m_query.isEmpty()) {
        clear();
    }

    if (m_query.isEmpty() && m_runnerManager) {
        return;
    }

    createManager();

    m_runnerManager->launchQuery(m_query);
}

void RunnerModel::matchesChanged(const QList<Plasma::QueryMatch> &matches)
{
    // Group matches by runner.
    // We do not use a QMultiHash here because it keeps values in LIFO order, while we want FIFO.
    QHash<QString, QList<Plasma::QueryMatch> > matchesForRunner;

    foreach (const Plasma::QueryMatch &match, matches) {
        auto it = matchesForRunner.find(match.runner()->id());

        if (it == matchesForRunner.end()) {
            it = matchesForRunner.insert(match.runner()->id(), QList<Plasma::QueryMatch>());
        }

        it.value().append(match);
    }

    // Sort matches for all runners in descending order. This allows the best
    // match to win whilest preserving order between runners.
    for (auto &list : matchesForRunner) {
        std::sort(list.begin(), list.end(), qGreater<Plasma::QueryMatch>());
    }

    if (m_mergeResults) {
        RunnerMatchesModel *matchesModel = nullptr;

        if (m_models.isEmpty()) {
            matchesModel = new RunnerMatchesModel(QString(), i18n("Search results"),
                m_runnerManager, this);

            beginInsertRows(QModelIndex(), 0, 0);
            m_models.append(matchesModel);
            endInsertRows();
            emit countChanged();
        } else {
            matchesModel = m_models.at(0);
        }

        QList<Plasma::QueryMatch> matches;

        foreach (const QString &runnerId, m_runners) {
            matches.append(matchesForRunner.take(runnerId));
        }

        matchesModel->setMatches(matches);

        return;
    }

    // Assign matches to existing models. If there is no match for a model, delete it.
    for (int row = m_models.count() - 1; row >= 0; --row) {
        RunnerMatchesModel *matchesModel = m_models.at(row);
        QList<Plasma::QueryMatch> matches = matchesForRunner.take(matchesModel->runnerId());

        if (m_deleteWhenEmpty && matches.isEmpty()) {
            beginRemoveRows(QModelIndex(), row, row);
            m_models.removeAt(row);
            delete matchesModel;
            endRemoveRows();
            emit countChanged();
        } else {
            matchesModel->setMatches(matches);
        }
    }

    // At this point, matchesForRunner contains only matches for runners which
    // do not have a model yet. Create new models for them.
    if (!matchesForRunner.isEmpty()) {
        auto it = matchesForRunner.constBegin();
        auto end = matchesForRunner.constEnd();
        int appendCount = 0;

        for (; it != end; ++it) {
            QList<Plasma::QueryMatch> matches = it.value();
            Q_ASSERT(!matches.isEmpty());
            RunnerMatchesModel *matchesModel = new RunnerMatchesModel(it.key(),
                matches.first().runner()->name(), m_runnerManager, this);
            matchesModel->setMatches(matches);

            if (it.key() == QLatin1String("services")) {
                beginInsertRows(QModelIndex(), 0, 0);
                m_models.prepend(matchesModel);
                endInsertRows();
                emit countChanged();
            } else {
                m_models.append(matchesModel);
                ++appendCount;
            }
        }

        if (appendCount > 0) {
            beginInsertRows(QModelIndex(), rowCount() - appendCount, rowCount() - 1);
            endInsertRows();
            emit countChanged();
        }
    }
}

void RunnerModel::createManager()
{
    if (!m_runnerManager) {
        m_runnerManager = new Plasma::RunnerManager(this); // FIXME: Which KConfigGroup is this using now?
        m_runnerManager->setAllowedRunners(m_runners);
        connect(m_runnerManager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)),
                this, SLOT(matchesChanged(QList<Plasma::QueryMatch>)));
    }
}

void RunnerModel::clear()
{
    if (m_runnerManager) {
        m_runnerManager->reset();
    }

    if (m_models.isEmpty()) {
        return;
    }

    beginResetModel();

    qDeleteAll(m_models);
    m_models.clear();

    endResetModel();

    emit countChanged();
}
