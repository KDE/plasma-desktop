/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>

class TouchpadConfigContainer;
class TouchpadBackend;
class KCModule;

class TouchpadConfigPlugin : public QWidget
{
    Q_OBJECT

public:
    TouchpadConfigPlugin(KCModule *parent, TouchpadBackend *backend);
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

protected:
    TouchpadConfigContainer *m_parent;
    TouchpadBackend *m_backend;
};
