/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WHEELINTERCEPTOR_H
#define WHEELINTERCEPTOR_H

#include <QPointer>
#include <QQuickItem>

class WheelInterceptor : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject *destination READ destination WRITE setDestination NOTIFY destinationChanged)

public:
    explicit WheelInterceptor(QQuickItem *parent = nullptr);
    ~WheelInterceptor() override;

    QObject *destination() const;
    void setDestination(QObject *destination);

Q_SIGNALS:
    void destinationChanged() const;

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QPointer<QObject> m_destination;
};

#endif
