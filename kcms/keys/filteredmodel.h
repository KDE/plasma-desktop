/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILTEREDMODEL_H
#define FILTEREDMODEL_H

#include <QSortFilterProxyModel>

class FilteredShortcutsModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    explicit FilteredShortcutsModel(QObject *parent);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QString filter() const;
    void setFilter(const QString &filter);

Q_SIGNALS:
    void filterChanged();

private:
    QString m_filter;
};
#endif
