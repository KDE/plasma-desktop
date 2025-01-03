/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QSortFilterProxyModel>

#include "kglobalaccelmodel_export.h"

class KGLOBALACCELMODEL_EXPORT FilteredShortcutsModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool showsLaunchAction READ showsLaunchAction WRITE setShowsLaunchAction NOTIFY showsLaunchActionChanged)

public:
    explicit FilteredShortcutsModel(QObject *parent);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QString filter() const;
    void setFilter(const QString &filter);

    bool showsLaunchAction() const;
    void setShowsLaunchAction(bool value);

Q_SIGNALS:
    void filterChanged();
    void showsLaunchActionChanged();

private:
    QString m_filter;
    bool m_showsLaunchAction = true;
};
