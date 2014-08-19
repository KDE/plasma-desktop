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

#ifndef RECENTDOCSMODEL_H
#define RECENTDOCSMODEL_H

#include "abstractentry.h"
#include "abstractmodel.h"

class DocEntry : public AbstractEntry
{
    public:
        DocEntry(const QString &name, const QString &icon,
            const QString &url, const QString &desktopPath);

        EntryType type() const { return RunnableType; }

        QString url() const { return m_url; }
        QString desktopPath() const { return m_desktopPath; }

    private:
        QString m_url;
        QString m_desktopPath;
};

class RecentDocsModel : public AbstractModel
{
    Q_OBJECT

    public:
        explicit RecentDocsModel(QObject *parent = 0);
        ~RecentDocsModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

    private Q_SLOTS:
        void refresh();

    private:
        void forget(int row);
        void forgetAll();

        QList<DocEntry *> m_entryList;
};

#endif
