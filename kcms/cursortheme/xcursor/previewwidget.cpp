/*
 * Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QPainter>
#include <QMouseEvent>
#include <QQuickRenderControl>
#include <QQuickWindow>

#include <KWindowSystem>

#include "previewwidget.h"

#include "cursortheme.h"



namespace {

    // Preview cursors
    const char * const cursor_names[] =
    {
        "left_ptr",
        "left_ptr_watch",
        "wait",
        "pointer",
        "help",
        "ibeam",
        "size_all",
        "size_fdiag",
        "cross",
        "split_h",
        "size_ver",
        "size_hor",
        "size_bdiag",
        "split_v",
    };

    const int numCursors      = 9;     // The number of cursors from the above list to be previewed
    const int cursorSpacing   = 20;    // Spacing between preview cursors
    const qreal widgetMinWidth  = 10;    // The minimum width of the preview widget
    const qreal widgetMinHeight = 48;    // The minimum height of the preview widget
}


class PreviewCursor
{
    public:
        PreviewCursor( const CursorTheme *theme, const QString &name, int size );

        const QPixmap &pixmap() const { return m_pixmap; }
        int width() const { return m_pixmap.width(); }
        int height() const { return m_pixmap.height(); }
        int boundingSize() const { return m_boundingSize; }
        inline QRect rect() const;
        void setPosition( const QPoint &p ) { m_pos = p; }
        void setPosition( int x, int y ) { m_pos = QPoint(x, y); }
        QPoint position() const { return m_pos; }
        operator const QPixmap& () const { return pixmap(); }

    private:
        int m_boundingSize;
        QPixmap m_pixmap;
        QPoint  m_pos;
};


PreviewCursor::PreviewCursor(const CursorTheme *theme, const QString &name, int size)
    : m_boundingSize(size > 0 ? size : theme->defaultCursorSize())
{
    // Create the preview pixmap
    QImage image = theme->loadImage(name, size);

    if (image.isNull())
        return;

    m_pixmap = QPixmap::fromImage(image);
}

QRect PreviewCursor::rect() const
{
    return QRect(m_pos, m_pixmap.size())
                .adjusted(-(cursorSpacing / 2), -(cursorSpacing / 2),
                          cursorSpacing / 2, cursorSpacing / 2);
}



// ------------------------------------------------------------------------------



PreviewWidget::PreviewWidget(QQuickItem *parent)
        : QQuickPaintedItem(parent),
          m_currentIndex(-1),
          m_currentSize(0)
{
    setAcceptHoverEvents(true);
    current = nullptr;
}


PreviewWidget::~PreviewWidget()
{
    qDeleteAll(list);
    list.clear();
}

void PreviewWidget::setThemeModel(SortProxyModel *themeModel)
{
    if (m_themeModel == themeModel) {
        return;
    }

    m_themeModel = themeModel;
    emit themeModelChanged();
}

SortProxyModel *PreviewWidget::themeModel()
{
    return m_themeModel;
}

void PreviewWidget::setCurrentIndex(int idx)
{
    if (m_currentIndex == idx) {
        return;
    }

    m_currentIndex = idx;
    emit currentIndexChanged();

    if (!m_themeModel) {
        return;
    }
    const CursorTheme *theme = m_themeModel->theme(m_themeModel->index(idx, 0));
    setTheme(theme, m_currentSize);
}

int PreviewWidget::currentIndex() const
{
    return m_currentIndex;
}

void PreviewWidget::setCurrentSize(int size)
{
    if (m_currentSize == size) {
        return;
    }

    m_currentSize = size;
    emit currentSizeChanged();

    if (!m_themeModel) {
        return;
    }
    const CursorTheme *theme = m_themeModel->theme(m_themeModel->index(m_currentIndex, 0));
    setTheme(theme, size);
}

int PreviewWidget::currentSize() const
{
    return m_currentSize;
}

void PreviewWidget::refresh()
{
    if (!m_themeModel) {
        return;
    }

    const CursorTheme *theme = m_themeModel->theme(m_themeModel->index(m_currentIndex, 0));
    setTheme(theme, m_currentSize);
}

void PreviewWidget::updateImplicitSize()
{
    qreal totalWidth = 0;
    qreal maxHeight = 0;

    foreach (const PreviewCursor *c, list)
    {
        totalWidth += c->width();
        maxHeight = qMax(c->height(), (int)maxHeight);
    }

    totalWidth += (list.count() - 1) * cursorSpacing;
    maxHeight = qMax(maxHeight, widgetMinHeight);

    setImplicitWidth(qMax(totalWidth, widgetMinWidth));
    setImplicitHeight(qMax(height(), maxHeight));
}


void PreviewWidget::layoutItems()
{
    if (!list.isEmpty())
    {
        const int spacing = 12;
        int nextX = spacing;
        int nextY = spacing;

        foreach (PreviewCursor *c, list)
        {
            c->setPosition(nextX, nextY);
            nextX += c->boundingSize() + spacing;
            if (nextX + c->boundingSize() > width()) {
                nextX = spacing;
                nextY += c->boundingSize() + spacing;
            }
        }
    }

    needLayout = false;
}


void PreviewWidget::setTheme(const CursorTheme *theme, const int size)
{
    qDeleteAll(list);
    list.clear();

    if (theme)
    {
        for (int i = 0; i < numCursors; i++)
            list << new PreviewCursor(theme, cursor_names[i], size);

        needLayout = true;
        updateImplicitSize();
    }

    current = nullptr;
    update();
}


void PreviewWidget::paint(QPainter *painter)
{
    if (needLayout)
        layoutItems();

    foreach(const PreviewCursor *c, list)
    {
        if (c->pixmap().isNull())
            continue;

        painter->drawPixmap(c->position(), *c);
    }
}


void PreviewWidget::hoverMoveEvent(QHoverEvent *e)
{
    if (needLayout)
        layoutItems();

    for (const PreviewCursor *c : qAsConst(list)) {
        if (c->rect().contains(e->pos())) {
            if (c != current) {
                setCursor(c->pixmap());
                current = c;
            }
            return;
        }
    }

    setCursor(Qt::ArrowCursor);
    current = nullptr;
}

void PreviewWidget::hoverLeaveEvent(QHoverEvent *e)
{
    Q_UNUSED(e);
    unsetCursor();
}

void PreviewWidget::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(newGeometry)
    Q_UNUSED(oldGeometry)
    if (!list.isEmpty()) {
        needLayout = true;
    }
}

