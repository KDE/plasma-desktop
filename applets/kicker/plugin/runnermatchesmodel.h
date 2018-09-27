/***************************************************************************
 *   Copyright (C) 2012 by Aurélien Gâteau <agateau@kde.org>               *
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

#ifndef RUNNERMATCHESMODEL_H
#define RUNNERMATCHESMODEL_H

#include "abstractmodel.h"

#include <KRunner/QueryMatch>

namespace Plasma {
    class RunnerManager;
}

class RunnerMatchesModel : public AbstractModel
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)

    public:
        explicit RunnerMatchesModel(const QString &runnerId, const QString &name,
            Plasma::RunnerManager *manager, QObject *parent = nullptr);

        QString description() const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;

        QString runnerId() const { return m_runnerId; }
        QString name() const { return m_name; }

        void setMatches(const QList<Plasma::QueryMatch> &matches);

        AbstractModel* favoritesModel() override;

    private:
        QString m_runnerId;
        QString m_name;
        Plasma::RunnerManager *m_runnerManager;
        QList<Plasma::QueryMatch> m_matches;
};

#endif
