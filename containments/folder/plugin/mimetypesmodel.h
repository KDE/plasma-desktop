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
#include <QSortFilterProxyModel>

class QStringList;

class MimeTypesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList checkedTypes READ checkedTypes WRITE setCheckedTypes NOTIFY checkedTypesChanged)

    public:
        MimeTypesModel(QObject *parent = 0);
        ~MimeTypesModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE void checkAll();
        Q_INVOKABLE void setRowChecked(int row, bool checked);

        int rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent) return m_mimeTypesList.size(); }

        QStringList checkedTypes() const;
        void setCheckedTypes(const QStringList &list);

    Q_SIGNALS:
        void checkedTypesChanged() const;

    private:
        int indexOfType(const QString &name) const;

        QList<QMimeType> m_mimeTypesList;
        QVector<bool> checkedRows;
};

class FilterableMimeTypesModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList checkedTypes READ checkedTypes WRITE setCheckedTypes NOTIFY checkedTypesChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

    public:
        FilterableMimeTypesModel(QObject *parent = 0);
        ~FilterableMimeTypesModel();

        Q_INVOKABLE void setRowChecked(int row, bool checked);
        Q_INVOKABLE void checkAll();

        QStringList checkedTypes() const;
        void setCheckedTypes(const QStringList &list);

        QString filter() const;
        void setFilter(const QString &filter);

    Q_SIGNALS:
        void checkedTypesChanged() const;
        void filterChanged() const;

    private:
        MimeTypesModel *m_sourceModel;
        QString m_filter;
};

#endif
