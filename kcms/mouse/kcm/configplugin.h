/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGPLUGIN_H
#define CONFIGPLUGIN_H

#include <QWidget>

class ConfigContainer;
class InputBackend;

class ConfigPlugin : public QWidget
{
    Q_OBJECT

public:
    static ConfigPlugin *implementation(ConfigContainer *parent);

    explicit ConfigPlugin(ConfigContainer *parent);
    virtual ~ConfigPlugin()
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
    ConfigContainer *m_parent;
    InputBackend *m_backend;
};

#endif // CONFIGPLUGIN_H
