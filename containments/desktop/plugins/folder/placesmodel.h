/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <QSortFilterProxyModel>

class KFilePlacesModel;

class PlacesModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool activityLinkingEnabled READ activityLinkingEnabled CONSTANT)
    Q_PROPERTY(bool showDesktopEntry READ showDesktopEntry WRITE setShowDesktopEntry NOTIFY showDesktopEntryChanged)

public:
    explicit PlacesModel(QObject *parent = nullptr);
    ~PlacesModel() override;

    bool activityLinkingEnabled() const;

    bool showDesktopEntry() const;
    void setShowDesktopEntry(bool showDesktopEntry);

    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QString urlForIndex(int idx) const;
    Q_INVOKABLE int indexForUrl(const QString &url) const;

Q_SIGNALS:
    void placesChanged() const;
    void showDesktopEntryChanged() const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    KFilePlacesModel *m_sourceModel;
    bool m_showDesktopEntry = true;
};

#endif
