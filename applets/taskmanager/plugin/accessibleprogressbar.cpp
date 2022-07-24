/*
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QT_NO_ACCESSIBILITY
#include "accessibleprogressbar.h"

#include <QQuickWindow>

AccessibleProgressBar::AccessibleProgressBar(QObject *parent)
    : m_object(parent)
{
}

QAccessibleInterface *AccessibleProgressBar::child(int) const
{
    return nullptr;
}

QAccessibleInterface *AccessibleProgressBar::childAt(int, int) const
{
    return nullptr;
}

int AccessibleProgressBar::childCount() const
{
    return 0;
}

int AccessibleProgressBar::indexOfChild(const QAccessibleInterface *) const
{
    return -1;
}

QWindow *AccessibleProgressBar::window() const
{
    return item()->window();
}

bool AccessibleProgressBar::isValid() const
{
    return !m_object.isNull();
}

QObject *AccessibleProgressBar::object() const
{
    return m_object.data();
}

QAccessibleInterface *AccessibleProgressBar::parent() const
{
    return QAccessible::queryAccessibleInterface(item()->parentItem());
}

QRect AccessibleProgressBar::rect() const
{
    // ### no window in some cases.
    // ### Should we really check for 0 opacity?
    if (!item()->window() || !item()->isVisible() || qFuzzyIsNull(item()->opacity())) {
        return QRect();
    }

    QSize itemSize((int)item()->width(), (int)item()->height());
    // ### If the bounding rect fails, we first try the implicit size, then we go for the
    // parent size. WE MIGHT HAVE TO REVISIT THESE FALLBACKS.
    if (itemSize.isEmpty()) {
        itemSize = QSize((int)item()->implicitWidth(), (int)item()->implicitHeight());
        if (itemSize.isEmpty() && item()->parentItem())
            // ### Seems that the above fallback is not enough, fallback to use the parent size...
            itemSize = QSize((int)item()->parentItem()->width(), (int)item()->parentItem()->height());
    }

    QPointF scenePoint = item()->mapToScene(QPointF(0, 0));
    QPoint screenPos = item()->window()->mapToGlobal(scenePoint.toPoint());
    return QRect(screenPos, itemSize);
}

QAccessible::State AccessibleProgressBar::state() const
{
    QAccessible::State state;

    if (!window() || !window()->isVisible() || !item()->isVisible() || qFuzzyIsNull(item()->opacity())) {
        state.invisible = true;
    }

    return state;
}

QAccessible::Role AccessibleProgressBar::role() const
{
    return QAccessible::ProgressBar;
}

QString AccessibleProgressBar::text(QAccessible::Text textType) const
{
    switch (textType) {
    case QAccessible::Value:
        return QString::number(item()->property("value").toInt());
    default:
        return QString();
    }
}

void AccessibleProgressBar::setText(QAccessible::Text textType, const QString &text)
{
    if (textType != QAccessible::Value) {
        return;
    }

    constexpr auto valuePropertyName = "value";
    if (object()->metaObject()->indexOfProperty(valuePropertyName) >= 0) {
        object()->setProperty(valuePropertyName, text);
    }
}

void *AccessibleProgressBar::interface_cast(QAccessible::InterfaceType type)
{
    if (type == QAccessible::ValueInterface) {
        return static_cast<QAccessibleValueInterface *>(this);
    }

    return nullptr;
}

QVariant AccessibleProgressBar::currentValue() const
{
    return item()->property("value");
}

QVariant AccessibleProgressBar::maximumValue() const
{
    return item()->property("to");
}

QVariant AccessibleProgressBar::minimumStepSize() const
{
    return 1;
}

QVariant AccessibleProgressBar::minimumValue() const
{
    return item()->property("from");
}

void AccessibleProgressBar::setCurrentValue(const QVariant &value)
{
    item()->setProperty("value", value);
}

void AccessibleProgressBar::setObject(QObject *object)
{
    m_object = object;
}

#endif
