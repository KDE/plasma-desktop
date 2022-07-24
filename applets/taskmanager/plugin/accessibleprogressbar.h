/*
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#ifndef QT_NO_ACCESSIBILITY
#include <QAccessibleObject>
#include <QQuickItem>

/**
 * This class is used to work around https://bugreports.qt.io/browse/QTBUG-105155
 */
class AccessibleProgressBar : public QAccessibleInterface, public QAccessibleValueInterface
{
public:
    explicit AccessibleProgressBar(QObject *parent);

    QAccessibleInterface *child(int index) const override;
    QAccessibleInterface *childAt(int x, int y) const override;
    int childCount() const override;
    int indexOfChild(const QAccessibleInterface *child) const override;

    QWindow *window() const override;

    bool isValid() const override;
    QObject *object() const override;

    QAccessibleInterface *parent() const override;

    QRect rect() const override;

    QAccessible::State state() const override;
    QAccessible::Role role() const override;
    QString text(QAccessible::Text) const override;
    void setText(QAccessible::Text, const QString &text) override;

    void *interface_cast(QAccessible::InterfaceType type) override;

    // QAccessibleValueInterface
    QVariant currentValue() const override;
    QVariant maximumValue() const override;
    QVariant minimumStepSize() const override;
    QVariant minimumValue() const override;
    void setCurrentValue(const QVariant &value) override;

    void setObject(QObject *object);

private:
    QQuickItem *item() const
    {
        return static_cast<QQuickItem *>(object());
    }

    QPointer<QObject> m_object = nullptr;
};

#endif // QT_NO_ACCESSIBILITY
