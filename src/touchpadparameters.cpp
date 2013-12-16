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

#include "touchpadparameters.h"

static KSharedConfig::Ptr savedDefaults()
{
    static KSharedConfig::Ptr p(KSharedConfig::openConfig(".touchpaddefaults",
                                                          KConfig::SimpleConfig,
                                                          "tmp"));
    return p;
}

TouchpadParameters::TouchpadParameters()
    : m_temporary(false)
{
    useDefaults(true);
    loadFrom(savedDefaults().data());
    useDefaults(false);
    loadFrom(config());
}

void TouchpadParameters::saveAsDefaults()
{
    Q_FOREACH(KConfigSkeletonItem *i, items()) {
        i->writeConfig(savedDefaults().data());
    }
    savedDefaults()->sync();
}

void TouchpadParameters::setTemporary(bool v)
{
    m_temporary = v;
}

void TouchpadParameters::writeConfig()
{
    if (!m_temporary) {
        KCoreConfigSkeleton::writeConfig();
    }
}

void TouchpadParameters::loadFrom(KConfig *config)
{
    Q_FOREACH(KConfigSkeletonItem *i, items()) {
        i->readConfig(config);
    }
}

void TouchpadParameters::loadFrom(KCoreConfigSkeleton *config)
{
    Q_FOREACH(KConfigSkeletonItem *i, items()) {
        KConfigSkeletonItem *j = config->findItem(i->name());
        if (j) {
            i->setProperty(j->property());
        }
    }
}
