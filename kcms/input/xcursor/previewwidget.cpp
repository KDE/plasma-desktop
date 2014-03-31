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

#include "previewwidget.h"

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <xcb/xcb.h>

#include "cursortheme.h"



namespace {

    // Preview cursors
    const char * const cursor_names[] =
    {
        "left_ptr",
        "left_ptr_watch",
        "wait",
        "pointing_hand",
        "whats_this",
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
    const int widgetMinWidth  = 10;    // The minimum width of the preview widget
    const int widgetMinHeight = 48;    // The minimum height of the preview widget
}


class PreviewCursor
{
    public:
        PreviewCursor( const CursorTheme *theme, const QString &name, int size );
        ~PreviewCursor();

        const QPixmap &pixmap() const { return m_pixmap; }
        int width() const { return m_pixmap.width(); }
        int height() const { return m_pixmap.height(); }
        inline QRect rect() const;
        void setPosition( const QPoint &p ) { m_pos = p; }
        void setPosition( int x, int y ) { m_pos = QPoint(x, y); }
        QPoint position() const { return m_pos; }
        operator const uint32_t () const { return m_cursor; }
        operator const QPixmap& () const { return pixmap(); }

    private:
        QPixmap m_pixmap;
        uint32_t m_cursor;
        QPoint  m_pos;
};


PreviewCursor::PreviewCursor(const CursorTheme *theme, const QString &name, int size)
{
    // Create the preview pixmap
    QImage image = theme->loadImage(name, size);

    if (image.isNull())
        return;

    m_pixmap = QPixmap::fromImage(image);

    // Load the cursor
    m_cursor = theme->loadCursor(name, size);
    // ### perhaps we should tag the cursor so it doesn't get
    //     replaced when a new theme is applied
}

PreviewCursor::~PreviewCursor()
{
    if (QX11Info::isPlatformX11() && m_cursor != XCB_CURSOR_NONE) {
        xcb_free_cursor(QX11Info::connection(), m_cursor);
    }
}

QRect PreviewCursor::rect() const
{
    return QRect(m_pos, m_pixmap.size())
                .adjusted(-(cursorSpacing / 2), -(cursorSpacing / 2),
                          cursorSpacing / 2, cursorSpacing / 2);
}



// ------------------------------------------------------------------------------



PreviewWidget::PreviewWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    current = NULL;
}


PreviewWidget::~PreviewWidget()
{
    qDeleteAll(list);
    list.clear();
}


QSize PreviewWidget::sizeHint() const
{
    int totalWidth = 0;
    int maxHeight = 0;

    foreach (const PreviewCursor *c, list)
    {
        totalWidth += c->width();
        maxHeight = qMax(c->height(), maxHeight);
    }

    totalWidth += (list.count() - 1) * cursorSpacing;
    maxHeight = qMax(maxHeight, widgetMinHeight);

    return QSize(qMax(totalWidth, widgetMinWidth), qMax(height(), maxHeight));
}


void PreviewWidget::layoutItems()
{
    if (!list.isEmpty())
    {
        QSize size = sizeHint();
        int cursorWidth = size.width() / list.count();
        int nextX = (width() - size.width()) / 2;

        foreach (PreviewCursor *c, list)
        {
            c->setPosition(nextX + (cursorWidth - c->width()) / 2,
                           (height() - c->height()) / 2);
            nextX += cursorWidth;
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
        updateGeometry();
    }

    current = NULL;
    update();
}


void PreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (needLayout)
        layoutItems();

    foreach(const PreviewCursor *c, list)
    {
        if (c->pixmap().isNull())
            continue;

        p.drawPixmap(c->position(), *c);
    }
}


void PreviewWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (needLayout)
        layoutItems();

    foreach (const PreviewCursor *c, list)
    {
        if (c->rect().contains(e->pos()))
        {
            if (c != current)
            {
                const uint32_t cursor = *c;
                if (QX11Info::isPlatformX11() && (cursor != XCB_CURSOR_NONE)) {
                    xcb_change_window_attributes(QX11Info::connection(), winId(), XCB_CW_CURSOR, &cursor);
                }
                current = c;
            }
            return;
        }
    }

    setCursor(Qt::ArrowCursor);
    current = NULL;
}


void PreviewWidget::resizeEvent(QResizeEvent *)
{
    if (!list.isEmpty())
        needLayout = true;
}

