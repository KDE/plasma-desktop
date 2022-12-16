/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QQuickItem>
#include <QVariantMap>

class QUrl;

class DragHelper : public QQuickItem
{
    Q_OBJECT

public:
    explicit DragHelper(QQuickItem *parent = nullptr);
    ~DragHelper() override;

    Q_INVOKABLE QVariantMap generateMimeData(const QString &mimeType, const QVariant &mimeData, const QUrl &url) const;
};
