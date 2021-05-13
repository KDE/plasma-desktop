/***************************************************************************
 *   Copyright (C) 2016, 2019 Kai Uwe Broulik <kde@privat.broulik.de>      *
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

#include "smartlauncheritem.h"

#include <KDesktopFile>
#include <KService>

using namespace SmartLauncher;

Item::Item(QObject *parent)
    : QObject(parent)
{
    m_backendPtr = s_backend.toStrongRef();
    if (!m_backendPtr) {
        QSharedPointer<Backend> backendSharedPtr(new Backend);
        s_backend = backendSharedPtr;
        m_backendPtr = s_backend.toStrongRef();
    }
}

QWeakPointer<Backend> Item::s_backend;

void Item::init()
{
    if (m_inited || m_storageId.isEmpty() || !m_backendPtr) {
        return;
    }

    connect(m_backendPtr.data(), &Backend::reloadRequested, this, [this](const QString &uri) {
        if (uri.isEmpty() || m_storageId == uri) {
            populate();
        }
    });

    connect(m_backendPtr.data(), &Backend::launcherRemoved, this, [this](const QString &uri) {
        if (uri.isEmpty() || m_storageId == uri) {
            clear();
        }
    });

    connect(m_backendPtr.data(), &Backend::countChanged, this, [this](const QString &uri, int count) {
        if (uri.isEmpty() || m_storageId == uri) {
            setCount(count);
        }
    });

    connect(m_backendPtr.data(), &Backend::countVisibleChanged, this, [this](const QString &uri, bool countVisible) {
        if (uri.isEmpty() || m_storageId == uri) {
            setCountVisible(countVisible);
        }
    });

    connect(m_backendPtr.data(), &Backend::progressChanged, this, [this](const QString &uri, int progress) {
        if (uri.isEmpty() || m_storageId == uri) {
            setProgress(progress);
        }
    });

    connect(m_backendPtr.data(), &Backend::progressVisibleChanged, this, [this](const QString &uri, bool progressVisible) {
        if (uri.isEmpty() || m_storageId == uri) {
            setProgressVisible(progressVisible);
        }
    });

    connect(m_backendPtr.data(), &Backend::urgentChanged, this, [this](const QString &uri, bool urgent) {
        if (uri.isEmpty() || m_storageId == uri) {
            setUrgent(urgent);
        }
    });

    m_inited = true;
}

void Item::populate()
{
    if (!m_backendPtr || m_storageId.isEmpty()) {
        return;
    }

    if (!m_backendPtr->hasLauncher(m_storageId)) {
        return;
    }

    setCount(m_backendPtr->count(m_storageId));
    setCountVisible(m_backendPtr->countVisible(m_storageId));
    setProgress(m_backendPtr->progress(m_storageId));
    setProgressVisible(m_backendPtr->progressVisible(m_storageId));
    setUrgent(m_backendPtr->urgent(m_storageId));
}

void Item::clear()
{
    setCount(0);
    setCountVisible(false);
    setProgress(0);
    setProgressVisible(false);
    setUrgent(false);
}

QUrl Item::launcherUrl() const
{
    return m_launcherUrl;
}

void Item::setLauncherUrl(const QUrl &launcherUrl)
{
    if (launcherUrl != m_launcherUrl) {
        m_launcherUrl = launcherUrl;
        Q_EMIT launcherUrlChanged(launcherUrl);

        m_storageId.clear();
        clear();

        if (launcherUrl.scheme() == QLatin1String("applications")) {
            const KService::Ptr service = KService::serviceByMenuId(launcherUrl.path());

            if (service && launcherUrl.path() == service->menuId()) {
                m_storageId = service->menuId();
            }
        }

        if (launcherUrl.isLocalFile() && KDesktopFile::isDesktopFile(launcherUrl.toLocalFile())) {
            KDesktopFile f(launcherUrl.toLocalFile());

            const KService::Ptr service = KService::serviceByStorageId(f.fileName());
            if (service) {
                m_storageId = service->storageId();
            }
        }

        if (m_storageId.isEmpty()) {
            return;
        }

        if (m_backendPtr) {
            // check if we have a mapping to a different desktop file
            const QString &overrideStorageId = m_backendPtr->unityMappingRules().value(m_storageId);
            if (!overrideStorageId.isEmpty()) {
                m_storageId = overrideStorageId;
            }
        }

        init();
        populate();
    }
}

int Item::count() const
{
    return m_count;
}

void Item::setCount(int count)
{
    if (m_count != count) {
        m_count = count;
        Q_EMIT countChanged(count);
    }
}

bool Item::countVisible() const
{
    return m_countVisible;
}

void Item::setCountVisible(bool countVisible)
{
    if (m_countVisible != countVisible) {
        m_countVisible = countVisible;
        Q_EMIT countVisibleChanged(countVisible);
    }
}

int Item::progress() const
{
    return m_progress;
}

void Item::setProgress(int progress)
{
    if (m_progress != progress) {
        m_progress = progress;
        Q_EMIT progressChanged(progress);
    }
}

bool Item::progressVisible() const
{
    return m_progressVisible;
}

void Item::setProgressVisible(bool progressVisible)
{
    if (m_progressVisible != progressVisible) {
        m_progressVisible = progressVisible;
        Q_EMIT progressVisibleChanged(progressVisible);
    }
}

bool Item::urgent() const
{
    return m_urgent;
}

void Item::setUrgent(bool urgent)
{
    if (m_urgent != urgent) {
        m_urgent = urgent;
        Q_EMIT urgentChanged(urgent);
    }
}
