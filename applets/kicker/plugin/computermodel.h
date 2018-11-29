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

class SimpleFavoritesModel;

class KConcatenateRowsProxyModel;
class KFilePlacesModel;

namespace Solid {
    class Device;
}

class FilteredPlacesModel : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        explicit FilteredPlacesModel(QObject *parent = nullptr);
        ~FilteredPlacesModel() override;

        QUrl url(const QModelIndex &index) const;
        bool isDevice(const QModelIndex &index) const;
        Solid::Device deviceForIndex(const QModelIndex &index) const;

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    private:
        KFilePlacesModel *m_placesModel;
};

class RunCommandModel : public AbstractModel
{
    Q_OBJECT

    public:
        RunCommandModel(QObject *parent = nullptr);
        ~RunCommandModel() override;

        QString description() const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;
};

class ComputerModel : public ForwardingModel
{
    Q_OBJECT

    Q_PROPERTY(int appNameFormat READ appNameFormat WRITE setAppNameFormat NOTIFY appNameFormatChanged)
    Q_PROPERTY(QObject* appletInterface READ appletInterface WRITE setAppletInterface NOTIFY appletInterfaceChanged)
    Q_PROPERTY(QStringList systemApplications READ systemApplications WRITE setSystemApplications NOTIFY systemApplicationsChanged)

    public:
        explicit ComputerModel(QObject *parent = nullptr);
        ~ComputerModel() override;

        QString description() const override;

        int appNameFormat() const;
        void setAppNameFormat(int format);

        QObject *appletInterface() const;
        void setAppletInterface(QObject *appletInterface);

        QStringList systemApplications() const;
        void setSystemApplications(const QStringList &apps);

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;

    Q_SIGNALS:
        void appNameFormatChanged() const;
        void appletInterfaceChanged() const;
        void systemApplicationsChanged() const;

    private Q_SLOTS:
        void onSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    private:
        KConcatenateRowsProxyModel *m_concatProxy;
        RunCommandModel *m_runCommandModel;
        SimpleFavoritesModel *m_systemAppsModel;
        FilteredPlacesModel *m_filteredPlacesModel;
        AppEntry::NameFormat m_appNameFormat;
        QObject *m_appletInterface;
};

#endif
