/*
 *   Copyright © 2008 Fredrik Höglund <fredrik@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#include "itemviewadapter.h"

#include <QModelIndex>
#include <QPalette>
#include <QSize>

ItemViewAdapter::ItemViewAdapter(QObject *parent)
    : KAbstractViewAdapter(parent)
    , m_adapterView(nullptr)
    , m_adapterModel(nullptr)
    , m_adapterIconSize(-1)
{
}

QAbstractItemModel *ItemViewAdapter::model() const
{
    return m_adapterModel;
}

QSize ItemViewAdapter::iconSize() const
{
    return QSize(m_adapterIconSize, m_adapterIconSize);
}

QPalette ItemViewAdapter::palette() const
{
    return QPalette();
}

QRect ItemViewAdapter::visibleArea() const
{
    return m_adapterVisibleArea;
}

QRect ItemViewAdapter::visualRect(const QModelIndex &index) const
{
    // FIXME TODO: Implemented on DND branch.

    Q_UNUSED(index)

    return QRect();
}

void ItemViewAdapter::connect(Signal signal, QObject *receiver, const char *slot)
{
    if (signal == ScrollBarValueChanged) {
        QObject::connect(this, SIGNAL(viewScrolled()), receiver, slot);
    } else if (signal == IconSizeChanged) {
        QObject::connect(this, SIGNAL(adapterIconSizeChanged()), receiver, slot);
    }
}

QAbstractItemModel *ItemViewAdapter::adapterModel() const
{
    return m_adapterModel;
}

QObject *ItemViewAdapter::adapterView() const
{
    return m_adapterView;
}

void ItemViewAdapter::setAdapterView(QObject *view)
{
    if (m_adapterView != view) {
        m_adapterView = view;

        Q_EMIT adapterViewChanged();
    }
}

void ItemViewAdapter::setAdapterModel(QAbstractItemModel *model)
{
    if (m_adapterModel != model) {
        m_adapterModel = model;

        Q_EMIT adapterModelChanged();
    }
}

int ItemViewAdapter::adapterIconSize() const
{
    return m_adapterIconSize;
}

void ItemViewAdapter::setAdapterIconSize(int size)
{
    if (m_adapterIconSize != size) {
        m_adapterIconSize = size;

        Q_EMIT adapterIconSizeChanged();
    }
}

QRect ItemViewAdapter::adapterVisibleArea() const
{
    return m_adapterVisibleArea;
}

void ItemViewAdapter::setAdapterVisibleArea(QRect rect)
{
    if (m_adapterVisibleArea != rect) {
        m_adapterVisibleArea = rect;

        Q_EMIT adapterVisibleAreaChanged();
    }
}
