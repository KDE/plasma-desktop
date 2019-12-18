/*
 * Copyright (c) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <KQuickAddons/ConfigModule>

#include <QKeySequence>

class QAction;

class SourcesModel;
class FilterProxyModel;

namespace NotificationManager {
class Settings;
}

class KCMNotifications : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(SourcesModel *sourcesModel READ sourcesModel CONSTANT)
    Q_PROPERTY(FilterProxyModel *filteredModel READ filteredModel CONSTANT)

    Q_PROPERTY(NotificationManager::Settings *settings READ settings CONSTANT)

    Q_PROPERTY(QKeySequence toggleDoNotDisturbShortcut
               READ toggleDoNotDisturbShortcut
               WRITE setToggleDoNotDisturbShortcut
               NOTIFY toggleDoNotDisturbShortcutChanged)

    // So it can show the respective settings module right away
    Q_PROPERTY(QString initialDesktopEntry READ initialDesktopEntry WRITE setInitialDesktopEntry NOTIFY initialDesktopEntryChanged)
    Q_PROPERTY(QString initialNotifyRcName READ initialNotifyRcName WRITE setInitialNotifyRcName NOTIFY initialNotifyRcNameChanged)
    Q_PROPERTY(QString initialEventId READ initialEventId WRITE setInitialEventId NOTIFY initialEventIdChanged)

public:
    KCMNotifications(QObject *parent, const QVariantList &args);
    ~KCMNotifications() override;

    SourcesModel *sourcesModel() const;
    FilterProxyModel *filteredModel() const;

    NotificationManager::Settings *settings() const;

    QKeySequence toggleDoNotDisturbShortcut() const;
    void setToggleDoNotDisturbShortcut(const QKeySequence &shortcut);
    Q_SIGNAL void toggleDoNotDisturbShortcutChanged();

    QString initialDesktopEntry() const;
    void setInitialDesktopEntry(const QString &desktopEntry);

    QString initialNotifyRcName() const;
    void setInitialNotifyRcName(const QString &notifyRcName);

    QString initialEventId() const;
    void setInitialEventId(const QString &eventId);

    Q_INVOKABLE void configureEvents(const QString &notifyRcName, const QString &eventId, QQuickItem *ctx = nullptr);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

signals:
    void initialDesktopEntryChanged();
    void initialNotifyRcNameChanged();
    void initialEventIdChanged();

private:
    void processPendingDeletions();

    SourcesModel *m_sourcesModel;
    FilterProxyModel *m_filteredModel;

    NotificationManager::Settings *m_settings;

    QAction *m_toggleDoNotDisturbAction;
    QKeySequence m_toggleDoNotDisturbShortcut;
    bool m_toggleDoNotDisturbShortcutDirty = false;

    QString m_initialDesktopEntry;
    QString m_initialNotifyRcName;
    QString m_initialEventId;

};
