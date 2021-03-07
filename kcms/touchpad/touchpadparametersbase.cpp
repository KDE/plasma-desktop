/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

#include "touchpadparametersbase.h"
#include "touchpadbackend.h"

namespace
{
KConfigGroup &systemDefaults()
{
    static KSharedConfig::Ptr p(KSharedConfig::openConfig(".touchpaddefaults", KConfig::SimpleConfig, QStandardPaths::TempLocation));
    static KConfigGroup group(p->group("parameters"));
    return group;
}

}

TouchpadParametersBase::TouchpadParametersBase(const QString &configname, QObject *parent)
    : KCoreConfigSkeleton(configname, parent)
{
    if (!systemDefaults().exists()) {
        setSystemDefaults();
    }
}

QVariantHash TouchpadParametersBase::values() const
{
    QVariantHash r;
    const auto itemList = items();
    for (const KConfigSkeletonItem *skel : itemList) {
        r[skel->name()] = skel->property();
    }
    return r;
}

void TouchpadParametersBase::setValues(const QVariantHash &v)
{
    for (QVariantHash::ConstIterator i = v.begin(); i != v.end(); ++i) {
        KConfigSkeletonItem *j = findItem(i.key());
        if (j) {
            j->setProperty(i.value());
        }
    }
}

void TouchpadParametersBase::setSystemDefaults()
{
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (!backend) {
        return;
    }
    QVariantHash v;
    backend->getConfig(v);

    for (QVariantHash::ConstIterator i = v.constBegin(); i != v.constEnd(); ++i) {
        systemDefaults().writeEntry(i.key(), i.value());
    }
    systemDefaults().sync();
}

QVariant TouchpadParametersBase::systemDefault(const QString &name, const QVariant &hardcoded)
{
    return systemDefaults().readEntry(name, hardcoded);
}
