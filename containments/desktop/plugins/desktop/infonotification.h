/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                   *
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

#ifndef INFONOTIFICATION_H
#define INFONOTIFICATION_H

#include <QObject>
#include <QPointer>

class KNotification;

class InfoNotification : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(QString titleText READ titleText WRITE setTitleText NOTIFY titleTextChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString acknowledgeActionText READ acknowledgeActionText WRITE setAcknowledgeActionText NOTIFY acknowledgeActionTextChanged)

    public:
        explicit InfoNotification(QObject *parent = nullptr);
        ~InfoNotification() override;

        bool enabled() const;
        void setEnabled(bool enabled);

        QString iconName() const;
        void setIconName(const QString &iconName);

        QString titleText() const;
        void setTitleText(const QString &titleText);

        QString text() const;
        void setText(const QString &text);

        QString acknowledgeActionText() const;
        void setAcknowledgeActionText(const QString &acknowledgeActionText);

        Q_INVOKABLE void show();

    Q_SIGNALS:
        void acknowledged() const;
        void enabledChanged() const;
        void iconNameChanged() const;
        void titleTextChanged() const;
        void textChanged() const;
        void acknowledgeActionTextChanged() const;

    private:
        bool m_enabled;
        QString m_iconName;
        QString m_titleText;
        QString m_text;
        QString m_acknowledgeActionText;
        QPointer<KNotification> m_notification;
};

#endif
