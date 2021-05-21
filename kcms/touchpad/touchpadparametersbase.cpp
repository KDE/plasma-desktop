/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
