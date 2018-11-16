/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

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
