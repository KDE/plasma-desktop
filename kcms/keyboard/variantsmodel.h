// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>

class VariantsModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(QString shortName READ shortName WRITE setShortName NOTIFY shortNameChanged)

public:
    explicit VariantsModel(QObject *parent = nullptr);

    QString searchString() const;
    void setSearchString(const QString &searchString);

    QString shortName() const;
    void setShortName(const QString &shortName);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

Q_SIGNALS:
    void searchStringChanged();
    void shortNameChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchString = QStringLiteral("");
    QString getFullName(const QModelIndex &idx) const;
    QString m_shortName;
};
