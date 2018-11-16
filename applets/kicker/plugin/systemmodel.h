/***************************************************************************
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

#ifndef SYSTEMMODEL_H
#define SYSTEMMODEL_H

#include "abstractmodel.h"

class SystemEntry;

class SystemModel : public AbstractModel
{
    Q_OBJECT

    public:
        explicit SystemModel(QObject *parent = nullptr);
        ~SystemModel() override;

        QString description() const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;

    protected Q_SLOTS:
        void refresh() override;

    private:
        void init();

        QList<SystemEntry *> m_entryList;
};

#endif
