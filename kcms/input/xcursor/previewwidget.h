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

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

class CursorTheme;
class PreviewCursor;

class PreviewWidget : public QWidget
{
    public:
        PreviewWidget(QWidget *parent);
        ~PreviewWidget();

        void setTheme(const CursorTheme *theme, const int size);
        void setUseLables(bool);
        QSize sizeHint() const;

    protected:
        void paintEvent(QPaintEvent *);
        void mouseMoveEvent(QMouseEvent *);
        void resizeEvent(QResizeEvent *);

    private:
        void layoutItems();

        QList<PreviewCursor*> list;
        const PreviewCursor *current;
        bool needLayout:1;
};

#endif // PREVIEWWIDGET_H

