/***************************************************************************
 *   Copyright (C) 2012 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
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

#ifndef RECENTCONTACTSMODEL_H
#define RECENTCONTACTSMODEL_H

#include "forwardingmodel.h"

namespace KPeople {
    class PersonData;
}

class RecentContactsModel : public ForwardingModel
{
    Q_OBJECT

    public:
        explicit RecentContactsModel(QObject *parent = 0);
        ~RecentContactsModel();

        QString description() const Q_DECL_OVERRIDE;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) Q_DECL_OVERRIDE;

        bool hasActions() const Q_DECL_OVERRIDE;
        QVariantList actions() const Q_DECL_OVERRIDE;

    private Q_SLOTS:
        void refresh() Q_DECL_OVERRIDE;
        void buildCache();
        void personDataChanged();

    private:
        void insertPersonData(const QString &id, int row);

        QHash<QString, KPeople::PersonData *> m_idToData;
        QHash<KPeople::PersonData *, int> m_dataToRow;
};

#endif
