/*
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    virtual ~LibinputConfig() {}

    void load() override;
    void save() override;
    void defaults() override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void hideEvent(QHideEvent *) override {}

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
