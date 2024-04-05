/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>
#include <KPluginMetaData>

#include <QQuickWidget>

#include "touchpadbackend.h"

class TouchpadConfigPlugin;

class KCMTouchpad : public KCModule
{
    Q_OBJECT

public:
    explicit KCMTouchpad(QObject *parent, const KPluginMetaData &data);

    static void kcmInit();

    void load() override;
    void save() override;
    void defaults() override;

Q_SIGNALS:
    void showMessage(const QString message, int type = 3 /*Kirigami.MessageType.Error*/);

private Q_SLOTS:
    void onChange();
    void onTouchpadAdded(bool success);
    void onTouchpadRemoved(int index);

private:
    void hideErrorMessage();

    QQuickWidget *m_view;
    TouchpadBackend *m_backend;
    bool m_initError;
};
