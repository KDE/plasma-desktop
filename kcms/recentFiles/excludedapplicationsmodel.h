/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QAbstractListModel>

#include "kactivitymanagerd_plugins_settings.h"

/**
 * ExcludedApplicationsModel
 */
class ExcludedApplicationsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ExcludedApplicationsModel(QObject *parent = nullptr);
    ~ExcludedApplicationsModel() override;

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

public Q_SLOTS:
    void toggleApplicationBlocked(int index);

    void load();
    void save();
    void defaults();

private:
    struct ApplicationData {
        QString name;
        QString title;
        QString icon;
        bool blocked;
    };

    QList<ApplicationData> m_applications;
    KActivityManagerdPluginsSettings *m_settings;
};
