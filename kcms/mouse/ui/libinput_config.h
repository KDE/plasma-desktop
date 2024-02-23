/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

class QHideEvent;
class QQuickWidget;
class KMessageWidget;
class ConfigContainer;
class InputBackend;

#include <QWidget>

class LibinputConfig : public QWidget
{
    Q_OBJECT

public:
    explicit LibinputConfig(ConfigContainer *parent, InputBackend *backend);
    ~LibinputConfig()
    {
    }

    void load();
    void save();
    void defaults();

private Q_SLOTS:
    void onChange();
    void onDeviceAdded(bool success);
    void onDeviceRemoved(int index);

private:
    void hideErrorMessage();

    QQuickWidget *m_view;
    KMessageWidget *m_errorMessage;
    InputBackend *m_backend;
    ConfigContainer *m_parent;

    bool m_initError;
};
