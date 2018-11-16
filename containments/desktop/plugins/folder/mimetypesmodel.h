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

#ifndef MIMETYPESMODEL_H
#define MIMETYPESMODEL_H

#include <QAbstractListModel>
#include <QMimeType>

class QStringList;

class MimeTypesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList checkedTypes READ checkedTypes WRITE setCheckedTypes NOTIFY checkedTypesChanged)

    public:
        explicit MimeTypesModel(QObject *parent = nullptr);
        ~MimeTypesModel() override;

        QHash<int, QByteArray> roleNames() const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role) override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent) return m_mimeTypesList.size(); }

        QStringList checkedTypes() const;
        void setCheckedTypes(const QStringList &list);

    Q_SIGNALS:
        void checkedTypesChanged() const;

    private:
        int indexOfType(const QString &name) const;

        QList<QMimeType> m_mimeTypesList;
        QVector<bool> m_checkedRows;
};

#endif
