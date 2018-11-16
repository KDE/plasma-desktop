/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
 *  Copyright (C) 2017 Eike Hein <hein@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef LAUNCHFEEDBACK_H
#define LAUNCHFEEDBACK_H

#include <KQuickAddons/ConfigModule>

class LaunchFeedback : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(int busyCursorCurrentIndex READ busyCursorCurrentIndex WRITE setBusyCursorCurrentIndex NOTIFY busyCursorCurrentIndexChanged)

    Q_PROPERTY(bool taskManagerNotification READ taskManagerNotification WRITE setTaskManagerNotification NOTIFY taskManagerNotificationChanged)

    Q_PROPERTY(int notificationTimeout READ notificationTimeout WRITE setNotificationTimeout NOTIFY notificationTimeoutChanged)

    public:
        explicit LaunchFeedback(QObject* parent = nullptr, const QVariantList &list = QVariantList());
        ~LaunchFeedback() override;

        int busyCursorCurrentIndex() const;
        void setBusyCursorCurrentIndex(int index);

        bool taskManagerNotification() const;
        void setTaskManagerNotification(bool enabled);

        int notificationTimeout() const;
        void setNotificationTimeout(int duration);

    public Q_SLOTS:
        void load() override;
        void save() override;
        void defaults() override;

    Q_SIGNALS:
        void busyCursorCurrentIndexChanged() const;

        void taskManagerNotificationChanged() const;

        void notificationTimeoutChanged() const;

    private:
        void updateNeedsSave();

        int m_busyCursorCurrentIndex;

        bool m_taskManagerNotification;

        int m_notificationTimeout;
};

#endif
