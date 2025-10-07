/*
    SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <PlasmaQuick/Dialog>
#include <qqmlregistration.h>

class SubDialog : public PlasmaQuick::Dialog
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool allowClosing READ allowClosing WRITE setAllowClosing NOTIFY allowClosingChanged)

public:
    explicit SubDialog(QQuickItem *parent = nullptr);
    ~SubDialog() override;

    Q_INVOKABLE QRect availableScreenRectForItem(QQuickItem *item) const;

    QPoint popupPosition(QQuickItem *item, const QSize &size) override;

    bool allowClosing() const;
    void setAllowClosing(bool allow);

Q_SIGNALS:
    void allowClosingChanged();

private:
    bool m_allowClosing = true;
};
