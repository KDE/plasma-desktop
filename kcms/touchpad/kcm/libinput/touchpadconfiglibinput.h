/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include "touchpadconfigcontainer.h"

class TouchpadBackend;
class QHideEvent;
class QQuickWidget;

class TouchpadConfigLibinput : public QObject
{
    Q_OBJECT

public:
    explicit TouchpadConfigLibinput(TouchpadConfigContainer *parent, TouchpadBackend *backend);
    virtual ~TouchpadConfigLibinput()
    {
    }

    void load();
    void save();
    void defaults();

Q_SIGNALS:
    void showMessage(const QString message, int type = 3 /*Kirigami.MessageType.Error*/);

private Q_SLOTS:
    void onChange();
    void onTouchpadAdded(bool success);
    void onTouchpadRemoved(int index);

private:
    void hideErrorMessage();

    QQuickWidget *m_view;

    bool m_initError;
    TouchpadBackend *m_backend;
    TouchpadConfigContainer *m_parent;
};
