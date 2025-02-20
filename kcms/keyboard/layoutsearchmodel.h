/*
    SPDX-FileCopyrightText: 2025 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QSortFilterProxyModel>

class LayoutSearchModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    LayoutSearchModel(QObject *parent = nullptr);

    Q_INVOKABLE QString searchString() const
    {
        return m_searchString;
    };
    Q_INVOKABLE void setSearchString(QStringView searchString);

    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged);

signals:
    void searchStringChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QString m_searchString;
    QString getFullName(const QModelIndex &idx) const;
    int m_searchScoreRole;
};