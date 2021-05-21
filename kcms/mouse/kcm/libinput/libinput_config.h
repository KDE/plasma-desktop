/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIBINPUTCONFIG_H
#define LIBINPUTCONFIG_H

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

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void hideEvent(QHideEvent *) override
    {
    }

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

#endif // LIBINPUTCONFIG_H
