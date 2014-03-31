/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SYSTEMMODEL_H
#define SYSTEMMODEL_H

#include "kickoffproxymodel.h"

#include <QThread>

namespace Kickoff
{

struct UsageInfo {
    UsageInfo()
        : used(0),
          available(0)
     {}

    quint64 used;
    quint64 available;
};

class UsageFinder : public QThread
{
    Q_OBJECT

public:
    UsageFinder(QObject *parent);
    void add(int index, const QString &mountPoint);

Q_SIGNALS:
    void usageInfo(int index, const QString &mountPoint, const UsageInfo &usageInfo);

protected:
    void run();

private:
    QList<QPair<int, QString> > m_toCheck;
};

/**
 * Model which provides a tree of items for important system setup tools (eg. System Settings) ,
 * folders (eg. the user's home folder and the local network) and fixed and removable storage.
 */
class  SystemModel : public KickoffProxyModel
{
    Q_OBJECT

public:
    /** Constructs a new SystemModel with the specified parent. */
    SystemModel(QObject *parent = 0);
    virtual ~SystemModel();

    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &item) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    void refreshUsageInfo();
    void stopRefreshingUsageInfo();

private Q_SLOTS:
    void startUsageInfoFetch();
    void reloadApplications();
    void sourceDataChanged(const QModelIndex &start, const QModelIndex &end);
    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
    void setUsageInfo(int index, const QString &mountPoint, const UsageInfo &usageInfo);
    void usageFinderFinished();

private:
    class Private;
    Private * const d;
};

}

#endif // SYSTEMMODEL_H

