/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DRAGHELPER_H
#define DRAGHELPER_H

#include <QObject>

class QIcon;
class QQuickItem;
class QUrl;

class DragHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int dragIconSize READ dragIconSize WRITE setDragIconSize NOTIFY dragIconSizeChanged)

public:
    explicit DragHelper(QObject *parent = nullptr);
    ~DragHelper() override;

    int dragIconSize() const;
    void setDragIconSize(int size);

    Q_INVOKABLE bool isDrag(int oldX, int oldY, int newX, int newY) const;
    Q_INVOKABLE void startDrag(QQuickItem *item, const QString &mimeType, const QVariant &mimeData, const QUrl &url, const QIcon &icon);

Q_SIGNALS:
    void dragIconSizeChanged() const;
    void dropped() const;

private Q_SLOTS:
    void startDragInternal(QQuickItem *item, const QString &mimeType, const QVariant &mimeData, const QUrl &url, const QIcon &icon) const;

private:
    int m_dragIconSize;
};

#endif
