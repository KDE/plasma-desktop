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

#ifndef RUNNERMODEL_H
#define RUNNERMODEL_H

#include <QAbstractListModel>
#include <QTimer>

#include <KRunner/QueryMatch>

namespace Plasma {
    class RunnerManager;
}

class RunnerMatchesModel;

class RunnerModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QStringList runners READ runners WRITE setRunners NOTIFY runnersChanged);
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged);

    public:
        explicit RunnerModel(QObject *parent = 0);
        ~RunnerModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int count() const;

        Q_INVOKABLE QObject *modelForRow(int row);

        QStringList runners() const;
        void setRunners(const QStringList &runners);

        QString query() const;
        void setQuery(const QString &query);

    Q_SIGNALS:
        void countChanged() const;
        void runnersChanged() const;
        void queryChanged() const;
        void appLaunched(const QString& storageId) const;

    private Q_SLOTS:
        void startQuery();
        void matchesChanged(const QList<Plasma::QueryMatch> &matches);

    private:
        void createManager();
        void clear();

        Plasma::RunnerManager *m_runnerManager;
        QStringList m_runners;
        QList<RunnerMatchesModel *> m_models;
        QString m_query;
        QTimer m_queryTimer;
};

#endif
