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
        explicit RecentContactsModel(QObject *parent = nullptr);
        ~RecentContactsModel() override;

        QString description() const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;

        bool hasActions() const override;
        QVariantList actions() const override;

    private Q_SLOTS:
        void refresh() override;
        void buildCache();
        void personDataChanged();

    private:
        void insertPersonData(const QString &id, int row);

        QHash<QString, KPeople::PersonData *> m_idToData;
        QHash<KPeople::PersonData *, int> m_dataToRow;
};

#endif
