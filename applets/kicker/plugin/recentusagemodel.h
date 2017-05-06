/***************************************************************************
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

#ifndef RECENTUSAGEMODEL_H
#define RECENTUSAGEMODEL_H

#include "forwardingmodel.h"

#include <QQmlParserStatus>
#include <QSortFilterProxyModel>

class GroupSortProxy : public QSortFilterProxyModel
{
    public:
        explicit GroupSortProxy(QAbstractItemModel *sourceModel);
        ~GroupSortProxy();

    protected:
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

class InvalidAppsFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        explicit InvalidAppsFilterProxy(AbstractModel *parentModel, QAbstractItemModel *sourceModel);
        ~InvalidAppsFilterProxy();

    protected:
        bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

    private Q_SLOTS:
        void connectNewFavoritesModel();

    private:
        QPointer<AbstractModel> m_parentModel;
};

class RecentUsageModel : public ForwardingModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int ordering READ ordering WRITE setOrdering NOTIFY orderingChanged)

    public:
        enum IncludeUsage { AppsAndDocs, OnlyApps, OnlyDocs };
        enum Ordering { Recent, Popular };

        explicit RecentUsageModel(
                QObject *parent = 0,
                IncludeUsage usage = AppsAndDocs,
                int ordering = Recent);
        ~RecentUsageModel();

        QString description() const;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

        bool hasActions() const;
        QVariantList actions() const;

        IncludeUsage usage() const;

        void setOrdering(int ordering);
        int ordering() const;

        void classBegin() override;
        void componentComplete() override;

    Q_SIGNALS:
        void orderingChanged(int ordering);

    private Q_SLOTS:
        void refresh();

    private:
        QVariant appData(const QString &resource, int role) const;
        QVariant docData(const QString &resource, int role) const;

        QString resourceAt(int row) const;

        QString forgetAllActionName() const;

        IncludeUsage m_usage;
        QPointer<QAbstractItemModel> m_activitiesModel;

        Ordering m_ordering;

        bool m_complete;
};

#endif
