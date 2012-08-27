/***************************************************************************
 *   Copyright (C) 2010 - 2011 by Ingomar Wesp <ingomar@wesp.name>         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/

#include "icongridlayout.h"

// KDE
#include <KIconLoader>

// stdlib
#include <math.h>

const int IconGridLayout::DEFAULT_CELL_SPACING = 4;

IconGridLayout::IconGridLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(parent),
      m_items(),
      m_mode(PreferRows),
      m_cellSpacing(DEFAULT_CELL_SPACING),
      m_maxSectionCount(0),
      m_maxSectionCountForced(false),
      m_rowCount(0),
      m_columnCount(0),
      m_rowHeights(),
      m_columnWidths()
{
    setContentsMargins(0, 0, 0, 0);

    QSizePolicy sizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHeightForWidth(true);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(1);
    setSizePolicy(sizePolicy);
    setMaximumSize(INT_MAX, INT_MAX);
}

IconGridLayout::~IconGridLayout()
{
    Q_FOREACH(QGraphicsLayoutItem * item, m_items) {
        if (item->ownedByLayout()) {
            delete item;
        }
    }
    m_items.clear();
}

IconGridLayout::Mode IconGridLayout::mode() const
{
    return m_mode;
}

void IconGridLayout::setMode(Mode mode)
{
    if (mode == m_mode) {
        return;
    }

    m_mode = mode;
    updateGridParameters();
    invalidate();
}

int IconGridLayout::cellSpacing() const
{
    return m_cellSpacing;
}

void IconGridLayout::setCellSpacing(int cellSpacing)
{
    cellSpacing = qMax(0, cellSpacing);

    if (cellSpacing == m_cellSpacing) {
        return;
    }

    m_cellSpacing = cellSpacing;
    updateGridParameters();
    invalidate();
}

int IconGridLayout::maxSectionCount() const
{
    return m_maxSectionCount;
}

void IconGridLayout::setMaxSectionCount(int maxSectionCount)
{
    if (m_maxSectionCount == maxSectionCount) {
        return;
    }

    m_maxSectionCount = maxSectionCount;
    updateGridParameters();
    invalidate();
}

bool IconGridLayout::maxSectionCountForced() const
{
    return m_maxSectionCountForced;
}

void IconGridLayout::setMaxSectionCountForced(bool enable)
{
    if (m_maxSectionCountForced == enable) {
        return;
    }

    m_maxSectionCountForced = enable;
    updateGridParameters();
    invalidate();
}

void IconGridLayout::addItem(QGraphicsLayoutItem *item)
{
    m_items.append(item);
    addChildLayoutItem(item);
    item->setParentLayoutItem(this);
    updateGridParameters();
    invalidate();
}

void IconGridLayout::insertItem(int index, QGraphicsLayoutItem *item)
{
    m_items.insert(index, item);
    addChildLayoutItem(item);
    item->setParentLayoutItem(this);
    updateGridParameters();
    invalidate();
}

void IconGridLayout::moveItem(int from, int to)
{
    m_items.move(from, to);
    invalidate();
}

int IconGridLayout::rowCount() const
{
    return m_rowCount;
}

int IconGridLayout::columnCount() const
{
    return m_columnCount;
}

int IconGridLayout::count() const
{
    return m_items.size();
}

QGraphicsLayoutItem *IconGridLayout::itemAt(int index) const
{
    return m_items[index];
}

QGraphicsLayoutItem *IconGridLayout::itemAt(int row, int column) const
{
    return m_items[row * m_columnCount + column];
}

void IconGridLayout::removeAt(int index)
{
    QGraphicsLayoutItem *item = m_items.takeAt(index);
    item->setParentLayoutItem(0);

    if (item->ownedByLayout()) {
        delete item;
    }

    updateGridParameters();
    invalidate();
}

void IconGridLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);
    updateGridParameters();

    qreal offsetLeft =
        qMax<qreal>(
            contentsRect().left(),
            (contentsRect().width() - preferredWidth()) / 2);

    qreal offsetTop =
        qMax<qreal>(
            contentsRect().top(),
            (contentsRect().height() - preferredHeight()) / 2);

    QPointF pos(offsetLeft, offsetTop);
    QSizeF size;

    int itemCount = m_items.size();
    for (int i = 0; i < itemCount; i++) {
        int row = i / m_columnCount;
        int column = i % m_columnCount;

        if (column == 0) {
            size.setHeight(m_rowHeights.at(row));
            if (row > 0) {
                pos.rx() = offsetLeft;
                pos.ry() += m_rowHeights.at(row - 1) + m_cellSpacing;
            }
        } else {
            pos.rx() += m_columnWidths.at(column - 1) + m_cellSpacing;
        }

        size.setWidth(m_columnWidths.at(column));
        m_items[i]->setGeometry(QRectF(pos, size));
    }
}

QSizeF IconGridLayout::sizeHint(
    Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint);

    switch (which) {
    case Qt::PreferredSize: return m_preferredSizeHint;
    case Qt::MinimumSize:
        if (m_mode == PreferRows) {
            return QSizeF(m_preferredSizeHint.width(), KIconLoader::SizeSmall);
        } else {
            return QSizeF(KIconLoader::SizeSmall, m_preferredSizeHint.height());
        }

    default:
        return QSizeF();
    }
}

void IconGridLayout::computeGridParameters(
    QList<int> &rowHeights, QList<int> &columnWidths,
    QSizeF &preferredSize) const
{
    columnWidths.clear();
    rowHeights.clear();

    const int itemCount = m_items.size();
    if (itemCount == 0) {
        preferredSize.setWidth(.0);
        preferredSize.setHeight(.0);
        return;
    }

    int rowCount;
    int columnCount;

    if (m_mode == PreferRows) {
        const int height = int(contentsRect().height());
        int minRowHeight = 0;
        Q_FOREACH(QGraphicsLayoutItem * item, m_items) {
            minRowHeight = qMax(minRowHeight, (int)item->minimumHeight());
        }

        if (m_maxSectionCount > 0 && m_maxSectionCountForced) {
            rowCount = qMin(itemCount, m_maxSectionCount);
        } else {
            rowCount = height / (minRowHeight + m_cellSpacing);
            rowCount = qBound(1, rowCount, itemCount);

            if (m_maxSectionCount > 0) {
                rowCount = qMin(rowCount, m_maxSectionCount);
            }
        }
        columnCount = ceil(double(itemCount) / rowCount);

        // Determine row heights.
        int maxRowHeight =
            qMax(minRowHeight,
                 (height - (rowCount - 1) * m_cellSpacing) / rowCount);

        for (int row = 0; row < rowCount; row++) {
            int desiredRowHeight = 0;
            for (int column = 0; column < columnCount; column++) {
                int itemIndex = row * columnCount + column;

                if (itemIndex >= itemCount) {
                    break;
                }
                desiredRowHeight =
                    qMax(desiredRowHeight, int(m_items.at(itemIndex)->preferredHeight()));
            }
            rowHeights.append(desiredRowHeight < maxRowHeight ? desiredRowHeight : maxRowHeight);
        }

        // Determine column widths.
        for (int column = 0; column < columnCount; column++) {
            int columnWidth = 0;
            for (int row = 0; row < rowCount; row++) {
                int itemIndex = row * columnCount + column;

                if (itemIndex >= itemCount) {
                    break;
                }

                int preferredItemWidth =
                    int(m_items.at(itemIndex)->effectiveSizeHint(Qt::PreferredSize, QSizeF(-1, rowHeights.at(row))).width());

                columnWidth = qMax(columnWidth, preferredItemWidth);

            }
            columnWidths.append(columnWidth);
        }
    } else { // m_mode == PreferColumns
        const int width = int(contentsRect().width());
        int minColumnWidth = 0;
        Q_FOREACH(QGraphicsLayoutItem * item, m_items) {
            minColumnWidth = qMax(minColumnWidth, (int)item->minimumWidth());
        }

        if (m_maxSectionCount > 0 && m_maxSectionCountForced) {
            columnCount = qMin(itemCount, m_maxSectionCount);
        } else {
            columnCount = width / (minColumnWidth + m_cellSpacing);
            columnCount = qBound(1, columnCount, itemCount);

            if (m_maxSectionCount > 0) {
                columnCount = qMin(columnCount, m_maxSectionCount);
            }
        }
        rowCount = ceil(double(itemCount) / columnCount);

        // Determine column widths.
        int maxColumnWidth =
            qMax(minColumnWidth,
                 (width - (columnCount - 1) * m_cellSpacing) / columnCount);

        for (int column = 0; column < columnCount; column++) {

            int desiredColumnWidth = 0;
            for (int row = 0; row < rowCount; row++) {
                int itemIndex = row * columnCount + column;

                if (itemIndex >= itemCount) {
                    break;
                }
                desiredColumnWidth =
                    qMax(desiredColumnWidth, int(m_items.at(itemIndex)->preferredWidth()));
            }
            columnWidths.append(desiredColumnWidth < maxColumnWidth ? desiredColumnWidth : maxColumnWidth);
        }

        // Determine row heights.
        for (int row = 0; row < rowCount; row++) {
            int rowHeight = 0;
            for (int column = 0; column < columnCount; column++) {
                int itemIndex = row * columnCount + column;
                if (itemIndex >= itemCount) {
                    break;
                }

                int preferredItemHeight =
                    int(m_items.at(itemIndex)->effectiveSizeHint(Qt::PreferredSize, QSizeF(columnWidths.at(column), -1)).height());

                rowHeight = qMax(rowHeight, preferredItemHeight);
            }
            rowHeights.append(rowHeight);
        }
    }

    Q_ASSERT(rowCount > 0 && columnCount > 0);
    Q_ASSERT(rowHeights.length() == rowCount && columnWidths.length() == columnCount);

    preferredSize.setWidth((columnCount - 1) * m_cellSpacing);
    preferredSize.setHeight((rowCount - 1) * m_cellSpacing);

    for (int i = 0; i < columnCount; i++) {
        preferredSize.rwidth() += columnWidths.at(i);
    }
    for (int i = 0; i < rowCount; i++) {
        preferredSize.rheight() += rowHeights.at(i);
    }
}

void IconGridLayout::updateGridParameters()
{
    QSizeF newPreferredSize;

    computeGridParameters(
        m_rowHeights, m_columnWidths,
        newPreferredSize);

    m_rowCount = m_rowHeights.size();
    m_columnCount = m_columnWidths.size();

    if (newPreferredSize != m_preferredSizeHint) {
        m_preferredSizeHint = newPreferredSize;
        updateGeometry();
    }
}
// vim: sw=4 sts=4 et tw=100
