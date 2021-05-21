/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TOUCHPADCONFIGPLUGIN_H
#define TOUCHPADCONFIGPLUGIN_H

#include <QWidget>

class TouchpadConfigContainer;
class TouchpadBackend;

class TouchpadConfigPlugin : public QWidget
{
    Q_OBJECT

public:
    TouchpadConfigPlugin(QWidget *parent, TouchpadBackend *backend);
    virtual ~TouchpadConfigPlugin()
    {
    }

    virtual void load()
    {
    }
    virtual void save()
    {
    }
    virtual void defaults()
    {
    }

    void hideEvent(QHideEvent *) override
    {
    }

protected:
    TouchpadConfigContainer *m_parent;
    TouchpadBackend *m_backend;
};

#endif // TOUCHPADCONFIGPLUGIN_H
