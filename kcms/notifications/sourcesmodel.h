/*
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

#include <QAbstractItemModel>
#include <QHash>
#include <QString>
#include <QVector>

struct EventData
{
    QString name;
    QString comment;
    QString iconName;
    QString eventId;
    QStringList actions;
};

// FIXME add constructors for KService and KConfigGroup
struct SourceData
{
    QString name;
    QString comment;
    QString iconName;

    QString notifyRcName;
    QString desktopEntry;

    QVector<EventData> events;

    QString display() const
    {
        return !name.isEmpty() ? name : comment;
    }
};

class SourcesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    SourcesModel(QObject *parent = nullptr);
    ~SourcesModel() override;

    enum Roles {
        SourceTypeRole = Qt::UserRole + 1,
        NotifyRcNameRole,
        DesktopEntryRole,

        EventIdRole,
        ActionsRole
    };
    Q_ENUM(Roles)

    enum Type {
        ApplicationType,
        ServiceType
    };
    Q_ENUM(Type)

    Q_INVOKABLE QPersistentModelIndex makePersistentModelIndex(const QModelIndex &idx) const;

    Q_INVOKABLE QPersistentModelIndex persistentIndexForDesktopEntry(const QString &desktopEntry) const;
    Q_INVOKABLE QPersistentModelIndex persistentIndexForNotifyRcName(const QString &notifyRcName) const;

    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

private:
    QVector<SourceData> m_data;

};
