/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splititem.h"
#include "splitnode.h"

SplitItem::SplitItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

QQuickItem *SplitItem::firstItem() const
{
    return m_firstItem;
}

void SplitItem::setFirstItem(QQuickItem *item)
{
    if (m_firstItem != item) {
        m_firstItem = item;
        update();
        Q_EMIT firstItemChanged();
    }
}

QQuickItem *SplitItem::secondItem() const
{
    return m_secondItem;
}

void SplitItem::setSecondItem(QQuickItem *item)
{
    if (m_secondItem != item) {
        m_secondItem = item;
        update();
        Q_EMIT secondItemChanged();
    }
}

qreal SplitItem::shutter() const
{
    return m_shutter;
}

void SplitItem::setShutter(qreal shutter)
{
    if (m_shutter != shutter) {
        m_shutter = shutter;
        update();
        Q_EMIT shutterChanged();
    }
}

QSGNode *SplitItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_firstItem || !m_firstItem->isTextureProvider()) {
        delete oldNode;
        return nullptr;
    }

    if (!m_secondItem || !m_secondItem->isTextureProvider()) {
        delete oldNode;
        return nullptr;
    }

    if (window()->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        SoftwareSplitNode *node = static_cast<SoftwareSplitNode *>(oldNode);
        if (!node) {
            node = new SoftwareSplitNode();
        }

        node->setRect(boundingRect());
        node->setWindow(window());
        node->setFirst(m_firstItem->textureProvider());
        node->setSecond(m_secondItem->textureProvider());

        return node;
    } else {
        SplitNode *node = static_cast<SplitNode *>(oldNode);
        if (!node) {
            node = new SplitNode();
        }

        node->setShutter(m_shutter);
        node->setFirst(m_firstItem->textureProvider());
        node->setSecond(m_secondItem->textureProvider());
        node->setRect(boundingRect());

        return node;
    }
}

#include "moc_splititem.cpp"
