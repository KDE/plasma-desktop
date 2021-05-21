/*
    SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBDIALOG_H
#define SUBDIALOG_H

#include <PlasmaQuick/Dialog>

class SubDialog : public PlasmaQuick::Dialog
{
    Q_OBJECT

public:
    explicit SubDialog(QQuickItem *parent = nullptr);
    ~SubDialog() override;

    Q_INVOKABLE QRect availableScreenRectForItem(QQuickItem *item) const;

    QPoint popupPosition(QQuickItem *item, const QSize &size) override;
};

#endif
