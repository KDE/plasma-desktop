/*
 * Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 * Copyright 2019 Kai Uwe Broulik <kde@broulik.de>
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

#pragma once

#include <QPointer>
#include <QQuickPaintedItem>
#include <QScopedPointer>

#include "ui_stylepreview.h"

class QWidget;
class QStyle;

class PreviewItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QString styleName READ styleName WRITE setStyleName NOTIFY styleNameChanged)

    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)

public:
    explicit PreviewItem(QQuickItem *parent = nullptr);
    ~PreviewItem() override;

    QString styleName() const;
    void setStyleName(const QString &styleName);
    Q_SIGNAL void styleNameChanged();

    bool isValid() const;
    Q_SIGNAL void validChanged();

    Q_INVOKABLE void reload();

    void componentComplete() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    void paint(QPainter *painter) override;

    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;

    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void sendHoverEvent(QHoverEvent *event);
    void dispatchEnterLeave(QWidget *enter, QWidget *leave, const QPointF &globalPosF);

    QString m_styleName;

    Ui::StylePreview m_ui;

    QScopedPointer<QWidget> m_widget;
    QPointer<QWidget> m_lastWidgetUnderMouse;

    QScopedPointer<QStyle> m_style;
};
