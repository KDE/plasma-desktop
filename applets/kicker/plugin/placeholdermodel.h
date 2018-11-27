/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
 *   Copyright (C) 2017 by Ivan Cukic <ivan.cukic@kde.org>                 *
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

#ifndef PLACEHOLDERMODEL_H
#define PLACEHOLDERMODEL_H

#include "abstractmodel.h"

#include <QPointer>
#include <QTimer>

class PlaceholderModel : public AbstractModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
    Q_PROPERTY(int dropPlaceholderIndex READ dropPlaceholderIndex WRITE setDropPlaceholderIndex NOTIFY dropPlaceholderIndexChanged)

    public:
        explicit PlaceholderModel(QObject *parent = nullptr);
        ~PlaceholderModel() override;

        QString description() const override;

        QAbstractItemModel *sourceModel() const;
        virtual void setSourceModel(QAbstractItemModel *sourceModel);

        bool canFetchMore(const QModelIndex &parent) const override;
        void fetchMore(const QModelIndex &parent) override;

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;

        Q_INVOKABLE QString labelForRow(int row) override;

        Q_INVOKABLE AbstractModel *modelForRow(int row) override;

        AbstractModel* favoritesModel() override;

        int separatorCount() const override;

        int dropPlaceholderIndex() const;
        void setDropPlaceholderIndex(int index);

    public Q_SLOTS:
        void reset();

    Q_SIGNALS:
        void sourceModelChanged() const;
        void dropPlaceholderIndexChanged();

    protected:
        void inhibitTriggering();

    private:
        QModelIndex indexToSourceIndex(const QModelIndex &index) const;
        QModelIndex sourceIndexToIndex(const QModelIndex &index) const;
        int sourceRowToRow(int sourceRow) const;
        int rowToSourceRow(int row) const;

        void connectSignals();
        void disconnectSignals();

        QPointer<QAbstractItemModel> m_sourceModel;

        int m_dropPlaceholderIndex;
        bool m_isTriggerInhibited;
        QTimer m_triggerInhibitor;
};

#endif
