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

#ifndef ABSTRACTMODEL_H
#define ABSTRACTMODEL_H

#include <QAbstractListModel>

class AbstractEntry;

class AbstractModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int separatorCount READ separatorCount NOTIFY separatorCountChanged)
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
    Q_PROPERTY(AbstractModel* favoritesModel READ favoritesModel WRITE setFavoritesModel NOTIFY favoritesModelChanged)

    public:
        explicit AbstractModel(QObject *parent = nullptr);
        ~AbstractModel() override;

        QHash<int, QByteArray> roleNames() const override;

        virtual QString description() const = 0;

        int count() const;
        virtual int separatorCount() const;

        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        int iconSize() const;
        void setIconSize(int size);

        Q_INVOKABLE virtual bool trigger(int row, const QString &actionId, const QVariant &argument) = 0;

        Q_INVOKABLE virtual void refresh();

        Q_INVOKABLE virtual QString labelForRow(int row);

        Q_INVOKABLE virtual AbstractModel *modelForRow(int row);
        Q_INVOKABLE virtual int rowForModel(AbstractModel *model);

        virtual bool hasActions() const;
        virtual QVariantList actions() const;

        virtual AbstractModel* favoritesModel();
        virtual void setFavoritesModel(AbstractModel *model);
        AbstractModel* rootModel();

        virtual void entryChanged(AbstractEntry *entry);

    Q_SIGNALS:
        void descriptionChanged() const;
        void countChanged() const;
        void separatorCountChanged() const;
        void iconSizeChanged() const;
        void favoritesModelChanged() const;

    protected:
        AbstractModel *m_favoritesModel;

    private:
        int m_iconSize;
};

#endif
