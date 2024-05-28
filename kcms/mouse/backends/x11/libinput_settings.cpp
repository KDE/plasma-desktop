/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "libinput_settings.h"

#include <KConfigGroup>
#include <KSharedConfig>

using namespace Qt::StringLiterals;

template<>
bool LibinputSettings::load(QString key, bool defVal)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig(u"kcminputrc"_s);
    KConfigGroup group(kcminputPtr, u"Mouse"_s);

    return group.readEntry(key, defVal);
}

template<>
qreal LibinputSettings::load(QString key, qreal defVal)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig(u"kcminputrc"_s);
    KConfigGroup group(kcminputPtr, u"Mouse"_s);

    return group.readEntry(key, defVal);
}

template<>
void LibinputSettings::save(QString key, bool val)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig(u"kcminputrc"_s);
    KConfigGroup group(kcminputPtr, u"Mouse"_s);

    group.writeEntry(key, val);

    group.sync();
    kcminputPtr->sync();
}

template<>
void LibinputSettings::save(QString key, qreal val)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig(u"kcminputrc"_s);
    KConfigGroup group(kcminputPtr, u"Mouse"_s);

    group.writeEntry(key, val);

    group.sync();
    kcminputPtr->sync();
}
