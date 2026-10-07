/*
    SPDX-FileCopyrightText: 2025 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QSortFilterProxyModel>

class LayoutSearchModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)

public:
    explicit LayoutSearchModel(QObject *parent = nullptr);

    QString searchString() const;
    void setSearchString(const QString &searchString);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

Q_SIGNALS:
    void searchStringChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchString = QStringLiteral("");
    QString getFullName(const QModelIndex &idx) const;
};
