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

#include "abstractmodel.h"

#include <QAbstractListModel>
#include <QTimer>

#include <KRunner/QueryMatch>

namespace Plasma {
    class RunnerManager;
}

class AbstractModel;
class RunnerMatchesModel;

class RunnerModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(AbstractModel* favoritesModel READ favoritesModel WRITE setFavoritesModel NOTIFY favoritesModelChanged)
    Q_PROPERTY(QObject* appletInterface READ appletInterface WRITE setAppletInterface NOTIFY appletInterfaceChanged)
    Q_PROPERTY(QStringList runners READ runners WRITE setRunners NOTIFY runnersChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(bool mergeResults READ mergeResults WRITE setMergeResults NOTIFY mergeResultsChanged)
    Q_PROPERTY(bool deleteWhenEmpty READ deleteWhenEmpty WRITE setDeleteWhenEmpty NOTIFY deleteWhenEmptyChanged)

    public:
        explicit RunnerModel(QObject *parent = nullptr);
        ~RunnerModel() override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        QHash<int, QByteArray> roleNames() const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int count() const;

        Q_INVOKABLE QObject *modelForRow(int row);

        QStringList runners() const;
        void setRunners(const QStringList &runners);

        QString query() const;
        void setQuery(const QString &query);

        AbstractModel *favoritesModel() const;
        void setFavoritesModel(AbstractModel *model);

        QObject *appletInterface() const;
        void setAppletInterface(QObject *appletInterface);

        bool mergeResults() const;
        void setMergeResults(bool merge);

        bool deleteWhenEmpty() const;
        void setDeleteWhenEmpty(bool deleteWhenEmpty);

    Q_SIGNALS:
        void countChanged() const;
        void favoritesModelChanged() const;
        void appletInterfaceChanged() const;
        void runnersChanged() const;
        void queryChanged() const;
        void mergeResultsChanged() const;
        void deleteWhenEmptyChanged();

    private Q_SLOTS:
        void startQuery();
        void matchesChanged(const QList<Plasma::QueryMatch> &matches);

    private:
        void createManager();
        void clear();

        AbstractModel *m_favoritesModel;
        QObject *m_appletInterface;
        Plasma::RunnerManager *m_runnerManager;
        QStringList m_runners;
        QList<RunnerMatchesModel *> m_models;
        QString m_query;
        QTimer m_queryTimer;
        bool m_mergeResults;
        bool m_deleteWhenEmpty;
};

#endif
