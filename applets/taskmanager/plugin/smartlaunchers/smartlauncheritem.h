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

#ifndef SMARTLAUNCHER_ITEM_H
#define SMARTLAUNCHER_ITEM_H

#include <QObject>
#include <QSharedPointer>
#include <QUrl>
#include <QWeakPointer>

#include "smartlauncherbackend.h"

namespace SmartLauncher
{
class Item : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl launcherUrl READ launcherUrl WRITE setLauncherUrl NOTIFY launcherUrlChanged)

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool countVisible READ countVisible NOTIFY countVisibleChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool progressVisible READ progressVisible NOTIFY progressVisibleChanged)
    Q_PROPERTY(bool urgent READ urgent NOTIFY urgentChanged)

public:
    explicit Item(QObject *parent = nullptr);
    ~Item() override = default;

    QUrl launcherUrl() const;
    void setLauncherUrl(const QUrl &launcherUrl);

    int count() const;
    bool countVisible() const;
    int progress() const;
    bool progressVisible() const;
    bool urgent() const;

Q_SIGNALS:
    void launcherUrlChanged(const QUrl &launcherUrl);

    void countChanged(int count);
    void countVisibleChanged(bool countVisible);
    void progressChanged(int progress);
    void progressVisibleChanged(bool progressVisible);
    void urgentChanged(bool urgent);

private:
    void init();

    void populate();
    void clear();

    void setCount(int count);
    void setCountVisible(bool countVisible);
    void setProgress(int progress);
    void setProgressVisible(bool progressVisible);
    void setUrgent(bool urgent);

    static QWeakPointer<Backend> s_backend;

    QSharedPointer<Backend> m_backendPtr;

    QUrl m_launcherUrl;
    QString m_storageId;

    bool m_inited = false;

    int m_count = 0;
    bool m_countVisible = false;
    int m_progress = 0;
    bool m_progressVisible = false;
    bool m_urgent = false;
};

} // namespace SmartLauncher

#endif // SMARTLAUNCHER_ITEM_H
