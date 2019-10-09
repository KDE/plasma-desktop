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

#include <QQuickPaintedItem>
#include <QPointer>
#include "sortproxymodel.h"

class CursorTheme;
class PreviewCursor;

class PreviewWidget : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(SortProxyModel *themeModel READ themeModel WRITE setThemeModel NOTIFY themeModelChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int currentSize READ currentSize WRITE setCurrentSize NOTIFY currentSizeChanged)


    public:
        explicit PreviewWidget(QQuickItem *parent = nullptr);
        ~PreviewWidget() override;

        void setTheme(const CursorTheme *theme, const int size);
        void setUseLables(bool);
        void updateImplicitSize();

        void setThemeModel(SortProxyModel *themeModel);
        SortProxyModel *themeModel();

        void setCurrentIndex(int idx);
        int currentIndex() const;

        void setCurrentSize(int size);
        int currentSize() const;

        Q_INVOKABLE void refresh();

    Q_SIGNALS:
        void themeModelChanged();
        void currentIndexChanged();
        void currentSizeChanged();

    protected:
        void paint(QPainter *) override;
        void hoverMoveEvent(QHoverEvent *event) override;
        void hoverLeaveEvent(QHoverEvent *e) override;
        void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    private:
        void layoutItems();

        QList<PreviewCursor*> list;
        const PreviewCursor *current;
        bool needLayout:1;
        QPointer<SortProxyModel> m_themeModel;
        int m_currentIndex;
        int m_currentSize;
};

#endif // PREVIEWWIDGET_H

