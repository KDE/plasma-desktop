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
#include "libinput_settings.h"

#include <KConfigGroup>
#include <KSharedConfig>

template<>
bool LibinputSettings::load(QString key, bool defVal)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig("kcminputrc");
    KConfigGroup group(kcminputPtr, "Mouse");

    return group.readEntry(key, defVal);
}

template<>
qreal LibinputSettings::load(QString key, qreal defVal)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig("kcminputrc");
    KConfigGroup group(kcminputPtr, "Mouse");

    return group.readEntry(key, defVal);
}

template<>
void LibinputSettings::save(QString key, bool val)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig("kcminputrc");
    KConfigGroup group(kcminputPtr, "Mouse");

    group.writeEntry(key, val);

    group.sync();
    kcminputPtr->sync();
}

template<>
void LibinputSettings::save(QString key, qreal val)
{
    KSharedConfig::Ptr kcminputPtr = KSharedConfig::openConfig("kcminputrc");
    KConfigGroup group(kcminputPtr, "Mouse");

    group.writeEntry(key, val);

    group.sync();
    kcminputPtr->sync();
}
