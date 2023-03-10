/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "../configplugin.h"

class QHideEvent;
class QQuickWidget;
class KMessageWidget;

class LibinputConfig : public ConfigPlugin
{
    Q_OBJECT

public:
    explicit LibinputConfig(ConfigContainer *parent, InputBackend *backend);
    virtual ~LibinputConfig()
    {
    }

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void onChange();
    void onDeviceAdded(bool success);
    void onDeviceRemoved(int index);

private:
    void hideErrorMessage();

    QQuickWidget *m_view;
    KMessageWidget *m_errorMessage;

    bool m_initError;
};
