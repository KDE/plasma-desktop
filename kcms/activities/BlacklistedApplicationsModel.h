/*
 *   Copyright (C) 2012 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BLACKLISTED_APPLICATIONS_MODEL_H
#define BLACKLISTED_APPLICATIONS_MODEL_H

#include <QAbstractListModel>

#include <utils/d_ptr.h>

/**
 * BlacklistedApplicationsModel
 */
class BlacklistedApplicationsModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit BlacklistedApplicationsModel(QObject *parent = nullptr);
    ~BlacklistedApplicationsModel() override;

    enum Roles {
        ApplicationIdRole = Qt::UserRole + 1,
        BlockedApplicationRole
    };

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void changed(bool changed);
    void defaulted(bool isDefault);
    void enabledChanged(bool enabled);

public Q_SLOTS:
    void toggleApplicationBlocked(int index);

    void setEnabled(bool);
    bool enabled() const;

    void load();
    void save();
    void defaults();

private:
    D_PTR;
};

#endif // BLACKLISTED_APPLICATIONS_MODEL_H
