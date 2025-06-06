/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QQuickItem>

class SplitItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *first READ firstItem WRITE setFirstItem NOTIFY firstItemChanged)
    Q_PROPERTY(QQuickItem *second READ secondItem WRITE setSecondItem NOTIFY secondItemChanged)
    Q_PROPERTY(qreal shutter READ shutter WRITE setShutter NOTIFY shutterChanged)

public:
    explicit SplitItem(QQuickItem *parent = nullptr);

    QQuickItem *firstItem() const;
    void setFirstItem(QQuickItem *item);

    QQuickItem *secondItem() const;
    void setSecondItem(QQuickItem *item);

    qreal shutter() const;
    void setShutter(qreal shutter);

Q_SIGNALS:
    void firstItemChanged();
    void secondItemChanged();
    void shutterChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    QPointer<QQuickItem> m_firstItem;
    QPointer<QQuickItem> m_secondItem;
    qreal m_shutter = 0.0;
};
