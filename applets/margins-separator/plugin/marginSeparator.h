/*
    SPDX-FileCopyrightText: 2022 Tanbir Jishan <tantalising007@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Plasma/Applet>
#include <PlasmaQuick/AppletQuickItem>

namespace Plasma
{
class Containment;
}

class MarginSeparator;

class MarginSeparator : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(PlasmaQuick::AppletQuickItem *containment READ containmentGraphicObject CONSTANT)

public:
    MarginSeparator(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    PlasmaQuick::AppletQuickItem *containmentGraphicObject() const;
};
