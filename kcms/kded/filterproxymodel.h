/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QSortFilterProxyModel>

#include "kcmkded.h"

class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(KDEDConfig::ModuleStatus statusFilter WRITE setStatusFilter NOTIFY statusFilterChanged)

public:
    FilterProxyModel(QObject *parent = nullptr);
    ~FilterProxyModel() override;

    QString query() const;
    void setQuery(const QString &query);

    KDEDConfig::ModuleStatus statusFilter() const;
    void setStatusFilter(KDEDConfig::ModuleStatus statusFilter);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void queryChanged();
    void statusFilterChanged();

private:
    QString m_query;
    KDEDConfig::ModuleStatus m_statusFilter = KDEDConfig::UnknownStatus; // "all"
};
