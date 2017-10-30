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

#ifndef ITEMVIEWADAPTER_H
#define ITEMVIEWADAPTER_H

#include <QRect>

#include <KAbstractViewAdapter>

class ItemViewAdapter : public KAbstractViewAdapter
{
    Q_OBJECT

    Q_PROPERTY(QObject* adapterView READ adapterView WRITE setAdapterView NOTIFY adapterViewChanged)
    Q_PROPERTY(QAbstractItemModel* adapterModel READ adapterModel WRITE setAdapterModel NOTIFY adapterModelChanged)
    Q_PROPERTY(int adapterIconSize READ adapterIconSize WRITE setAdapterIconSize NOTIFY adapterIconSizeChanged)
    Q_PROPERTY(QRect adapterVisibleArea READ adapterVisibleArea WRITE setAdapterVisibleArea NOTIFY adapterVisibleAreaChanged)

    public:
        explicit ItemViewAdapter(QObject* parent = nullptr);

        QAbstractItemModel *model() const override;
        QSize iconSize() const override;
        QPalette palette() const override;
        QRect visibleArea() const override;
        QRect visualRect(const QModelIndex &index) const override;
        void connect(Signal signal, QObject *receiver, const char *slot) override;

        QObject *adapterView() const;
        void setAdapterView(QObject *view);

        QAbstractItemModel *adapterModel() const;
        void setAdapterModel(QAbstractItemModel *model);

        int adapterIconSize() const;
        void setAdapterIconSize(int size);

        QRect adapterVisibleArea() const;
        void setAdapterVisibleArea(QRect rect);

    Q_SIGNALS:
        void viewScrolled() const;
        void adapterViewChanged() const;
        void adapterModelChanged() const;
        void adapterIconSizeChanged() const;
        void adapterVisibleAreaChanged() const;

    private:
        QObject *m_adapterView;
        QAbstractItemModel *m_adapterModel;
        int m_adapterIconSize;
        QRect m_adapterVisibleArea;
};

#endif

