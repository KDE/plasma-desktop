/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                         *
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

#ifndef COMPUTERMODEL_H
#define COMPUTERMODEL_H

#include "forwardingmodel.h"
#include "appentry.h"

#include <QSortFilterProxyModel>
#include <Solid/StorageAccess>

class FavoritesModel;

class KConcatenateRowsProxyModel;
class KFilePlacesModel;

namespace Solid {
    class Device;
}

class FilteredPlacesModel : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        FilteredPlacesModel(QObject *parent = 0);
        ~FilteredPlacesModel();

        QUrl url(const QModelIndex &index) const;
        bool isDevice(const QModelIndex &index) const;
        Solid::Device deviceForIndex(const QModelIndex &index) const;

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    private:
        KFilePlacesModel *m_placesModel;
};

class RunCommandModel : public AbstractModel
{
    public:
        RunCommandModel(QObject *parent = 0);
        ~RunCommandModel();

        QString description() const;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);
};

class ComputerModel : public ForwardingModel
{
    Q_OBJECT

    Q_PROPERTY(int appNameFormat READ appNameFormat WRITE setAppNameFormat NOTIFY appNameFormatChanged)
    Q_PROPERTY(QStringList systemApplications READ systemApplications WRITE setSystemApplications NOTIFY systemApplicationsChanged)

    public:
        explicit ComputerModel(QObject *parent = 0);
        ~ComputerModel();

        QString description() const;

        int appNameFormat() const;
        void setAppNameFormat(int format);

        QStringList systemApplications() const;
        void setSystemApplications(const QStringList &apps);

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

    Q_SIGNALS:
        void appNameFormatChanged() const;
        void systemApplicationsChanged() const;

    private Q_SLOTS:
        void onSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    private:
        KConcatenateRowsProxyModel *m_concatProxy;
        RunCommandModel *m_runCommandModel;
        FavoritesModel *m_systemAppsModel;
        FilteredPlacesModel *m_filteredPlacesModel;
        AppEntry::NameFormat m_appNameFormat;
};

#endif
