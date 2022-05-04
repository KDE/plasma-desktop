/*
    SPDX-FileCopyrightText: 2020 Tanbir Jishan <tantalising007@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "marginSeparator.h"

#include <QDebug>
#include <QProcess>
#include <QtQml>

#include <Plasma/Containment>
#include <Plasma/Corona>

MarginSeparator::MarginSeparator(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
{
}

PlasmaQuick::AppletQuickItem *MarginSeparator::containmentGraphicObject() const
{
    return containment()->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();
}

K_PLUGIN_CLASS_WITH_JSON(MarginSeparator, "../package/metadata.json")

#include "marginSeparator.moc"
