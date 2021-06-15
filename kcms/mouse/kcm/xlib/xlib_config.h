/*
    SPDX-FileCopyrightText: 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
    SPDX-FileCopyrightText: 1999 Dirk A. Mueller <dmuell@gmx.net>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XLIBCONFIG_H
#define XLIBCONFIG_H

#include "../configplugin.h"
#include "backends/x11/evdev_settings.h"

#include "ui_kcmmouse.h"
#include <config-workspace.h>

#include <KCModule>

class X11EvdevBackend;

class XlibConfig : public ConfigPlugin, public Ui::KCMMouse
{
    Q_OBJECT
public:
    XlibConfig(ConfigContainer *parent, InputBackend *backend);
    ~XlibConfig() = default;

    static void kcmInit();

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void slotHandedChanged(int val);
    void slotScrollPolarityChanged();
    void checkAccess();
    void slotThreshChanged(int value);
    void slotDragStartDistChanged(int value);
    void slotWheelScrollLinesChanged(int value);

private:
    double getAccel();
    int getThreshold();
    Handed getHandedness();

    void setAccel(double);
    void setThreshold(int);
    void setHandedness(Handed);

    X11EvdevBackend *m_backend;
};

#endif // XLIBCONFIG_H
