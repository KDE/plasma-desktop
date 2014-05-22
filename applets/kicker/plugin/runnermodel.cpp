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

#include <KRunner/AbstractRunner>
#include <KRunner/RunnerManager>

RunnerModel::RunnerModel(QObject *parent) : QAbstractListModel(parent),
    m_runnerManager(0)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");

    setRoleNames(roles);

    m_queryTimer.setSingleShot(true);
    m_queryTimer.setInterval(10);
    connect(&m_queryTimer, SIGNAL(timeout()), this, SLOT(startQuery()));
}

RunnerModel::~RunnerModel()
{
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
        return 0;
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
        QString runnerId = match.runner()->id();
        auto it = matchesForRunner.find(runnerId);

        if (it == matchesForRunner.end()) {
            it = matchesForRunner.insert(runnerId, QList<Plasma::QueryMatch>());
        }

        it.value().append(match);
    }

    // Assign matches to existing models. If there is no match for a model, delete it.
    for (int row = m_models.count() - 1; row >= 0; --row) {
        RunnerMatchesModel *matchesModel = m_models.at(row);
        QList<Plasma::QueryMatch> matches = matchesForRunner.take(matchesModel->runnerId());

        matchesModel->setMatches(matches);
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
            QString name = matches.first().runner()->name();
            RunnerMatchesModel *matchesModel = new RunnerMatchesModel(it.key(), name, m_runnerManager, this);
            connect(matchesModel, SIGNAL(appLaunched(QString)), this, SIGNAL(appLaunched(QString)));
            matchesModel->setMatches(matches);

            if (it.key() == "services") {
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
