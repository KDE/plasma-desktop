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

#ifndef TOUCHPADPARAMETERSBASE_H
#define TOUCHPADPARAMETERSBASE_H

#include <QVariantHash>

#include <KConfigSkeleton>

class TouchpadParametersBase : public KCoreConfigSkeleton
{
public:
    explicit TouchpadParametersBase(const QString &configname = QString(), QObject *parent = nullptr);

    QVariantHash values() const;
    void setValues(const QVariantHash &);

    static void setSystemDefaults();
    static QVariant systemDefault(const QString &name, const QVariant &hardcoded = QVariant());
    template<typename T> static T systemDefault(const QString &name, const T &hardcoded = T())
    {
        return qvariant_cast<T>(systemDefault(name, QVariant(hardcoded)));
    }

    template<typename T> static T systemDefaultEnum(const QString &name, const T &hardcoded = T())
    {
        return static_cast<T>(systemDefault(name, static_cast<int>(hardcoded)));
    }
};

#endif // TOUCHPADPARAMETERSBASE_H
