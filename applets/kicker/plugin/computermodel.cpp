/***************************************************************************
 * Copyright (C) 2007 Kevin Ottens <ervin@kde.org>                         *
 * Copyright (C) 2015 by Eike Hein <hein@kde.org>                          *
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

#include "computermodel.h"
#include "actionlist.h"
#include "simplefavoritesmodel.h"

#include <QIcon>

#include <KAuthorized>
#include <KConcatenateRowsProxyModel>
#include <KFilePlacesModel>
#include <KLocalizedString>
#include <KRun>
#include <Solid/Device>

#include "krunner_interface.h"

FilteredPlacesModel::FilteredPlacesModel(QObject *parent) : QSortFilterProxyModel(parent)
, m_placesModel(new KFilePlacesModel(this))
{
    setSourceModel(m_placesModel);
    sort(0);
}

FilteredPlacesModel::~FilteredPlacesModel()
{
}

QUrl FilteredPlacesModel::url(const QModelIndex &index) const
{
    return KFilePlacesModel::convertedUrl(m_placesModel->url(mapToSource(index)));
}

bool FilteredPlacesModel::isDevice(const QModelIndex &index) const
{
    return m_placesModel->isDevice(mapToSource(index));
}

Solid::Device FilteredPlacesModel::deviceForIndex(const QModelIndex &index) const
{
    return m_placesModel->deviceForIndex(mapToSource(index));
}

bool FilteredPlacesModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = m_placesModel->index(sourceRow, 0, sourceParent);

    return !m_placesModel->isHidden(index)
        && !m_placesModel->data(index, KFilePlacesModel::FixedDeviceRole).toBool();
}

bool FilteredPlacesModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool lDevice = m_placesModel->isDevice(left);
    bool rDevice = m_placesModel->isDevice(right);

    if (lDevice && !rDevice) {
        return false;
    } else if (!lDevice && rDevice) {
        return true;
    }

    return (left.row() < right.row());
}

RunCommandModel::RunCommandModel(QObject *parent) : AbstractModel(parent)
{
}

RunCommandModel::~RunCommandModel()
{
}

QString RunCommandModel::description() const
{
    return QString();
}

QVariant RunCommandModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return i18n("Run Command...");
    } else if (role == Qt::DecorationRole) {
        return QIcon::fromTheme(QStringLiteral("system-run"));
    } else if (role == Kicker::DescriptionRole) {
        return i18n("Run a command or a search query");
    } else if (role == Kicker::GroupRole) {
        return i18n("Applications");
    }

    return QVariant();
}

int RunCommandModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (KAuthorized::authorize(QStringLiteral("run_command")) ? 1 : 0);
}

Q_INVOKABLE bool RunCommandModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    Q_UNUSED(actionId)
    Q_UNUSED(argument)

    if (row == 0 && KAuthorized::authorize(QStringLiteral("run_command"))) {
        org::kde::krunner::App krunner(QStringLiteral("org.kde.krunner"),
            QStringLiteral("/App"), QDBusConnection::sessionBus());
        krunner.display();

        return true;
    }

    return false;
}

ComputerModel::ComputerModel(QObject *parent) : ForwardingModel(parent)
, m_concatProxy(new KConcatenateRowsProxyModel(this))
, m_runCommandModel(new RunCommandModel(this))
, m_systemAppsModel(new SimpleFavoritesModel(this))
, m_filteredPlacesModel(new FilteredPlacesModel(this))
, m_appNameFormat(AppEntry::NameOnly)
, m_appletInterface(nullptr)
{
    connect(m_systemAppsModel, &SimpleFavoritesModel::favoritesChanged, this, &ComputerModel::systemApplicationsChanged);
    m_systemAppsModel->setFavorites(QStringList() << QStringLiteral("systemsettings.desktop"));

    m_concatProxy->addSourceModel(m_runCommandModel);
    m_concatProxy->addSourceModel(m_systemAppsModel);
    m_concatProxy->addSourceModel(m_filteredPlacesModel);

    setSourceModel(m_concatProxy);
}

ComputerModel::~ComputerModel()
{
}

QString ComputerModel::description() const
{
    return i18n("Computer");
}

int ComputerModel::appNameFormat() const
{
    return m_appNameFormat;
}

void ComputerModel::setAppNameFormat(int format)
{
    if (m_appNameFormat != (AppEntry::NameFormat)format) {
        m_appNameFormat = (AppEntry::NameFormat)format;

        m_systemAppsModel->refresh();

        emit appNameFormatChanged();
    }
}

QObject *ComputerModel::appletInterface() const
{
    return m_appletInterface;
}

void ComputerModel::setAppletInterface(QObject *appletInterface)
{
    if (m_appletInterface != appletInterface) {
        m_appletInterface = appletInterface;

        emit appletInterfaceChanged();
    }
}

QStringList ComputerModel::systemApplications() const
{
    return m_systemAppsModel->favorites();
}

void ComputerModel::setSystemApplications(const QStringList &apps)
{
    m_systemAppsModel->setFavorites(apps);
}

QVariant ComputerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QModelIndex sourceIndex = m_concatProxy->mapToSource(m_concatProxy->index(index.row(),
        index.column()));

    bool isPlace = (sourceIndex.model() == m_filteredPlacesModel);

    if (isPlace)  {
        if (role == Kicker::DescriptionRole) {
            if (m_filteredPlacesModel->isDevice(sourceIndex)) {
                Solid::Device device = m_filteredPlacesModel->deviceForIndex(sourceIndex);
                Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

                if (access) {
                    return access->filePath();
                } else {
                    return QString();
                }
            } else {
                const QUrl &url = m_filteredPlacesModel->url(sourceIndex);
                return url.toString(QUrl::PreferLocalFile);
            }
        } else if (role == Kicker::FavoriteIdRole) {
            if (!m_filteredPlacesModel->isDevice(sourceIndex)) {
                return m_filteredPlacesModel->url(sourceIndex);
            }
        } else if (role == Kicker::UrlRole) {
            return m_filteredPlacesModel->url(sourceIndex);
        } else if (role == Kicker::GroupRole) {
            return sourceIndex.data(KFilePlacesModel::GroupRole).toString();
        } else if (role == Qt::DisplayRole || role == Qt::DecorationRole) {
            return sourceIndex.data(role);
        }
    } else if (role == Kicker::GroupRole) {
        return i18n("Applications");
    } else {
        return sourceIndex.data(role);
    }

    return QVariant();
}

bool ComputerModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    const QModelIndex sourceIndex = m_concatProxy->mapToSource(m_concatProxy->index(row, 0));

    if (sourceIndex.model() == m_filteredPlacesModel) {
        const QUrl &url = m_filteredPlacesModel->url(sourceIndex);

        if (url.isValid()) {
            new KRun(url, nullptr);

            return true;
        }

        Solid::Device device = m_filteredPlacesModel->deviceForIndex(sourceIndex);
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

        if (access && !access->isAccessible()) {
            connect(access, &Solid::StorageAccess::setupDone, this, &ComputerModel::onSetupDone);
            access->setup();

            return true;
        }
    } else {
        AbstractModel *model = nullptr;

        if (sourceIndex.model() == m_systemAppsModel) {
            model = m_systemAppsModel;
        } else {
            model = m_runCommandModel;
        }

        return model->trigger(sourceIndex.row(), actionId, argument);
    }

    return false;
}

void ComputerModel::onSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
    Q_UNUSED(errorData);

    if (error != Solid::NoError) {
        return;
    }

    Solid::Device device(udi);
    Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

    Q_ASSERT(access);

    new KRun(QUrl::fromLocalFile(access->filePath()), nullptr);
}
