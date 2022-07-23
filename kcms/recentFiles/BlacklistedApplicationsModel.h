/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BLACKLISTED_APPLICATIONS_MODEL_H
#define BLACKLISTED_APPLICATIONS_MODEL_H

#include <QAbstractListModel>

#include <utils/d_ptr.h>

/**
 * BlacklistedApplicationsModel
 */
class BlacklistedApplicationsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit BlacklistedApplicationsModel(QObject *parent = nullptr);
    ~BlacklistedApplicationsModel() override;

    enum Roles {
        ApplicationIdRole = Qt::UserRole + 1,
        BlockedApplicationRole,
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
