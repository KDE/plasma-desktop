/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QVariantHash>

#include <KCoreConfigSkeleton>

class TouchpadParametersBase : public KCoreConfigSkeleton
{
public:
    explicit TouchpadParametersBase(const QString &configname = QString(), QObject *parent = nullptr);

    QVariantHash values() const;
    void setValues(const QVariantHash &);

    static void setSystemDefaults();
    static QVariant systemDefault(const QString &name, const QVariant &hardcoded = QVariant());
    template<typename T>
    static T systemDefault(const QString &name, const T &hardcoded = T())
    {
        return qvariant_cast<T>(systemDefault(name, QVariant(hardcoded)));
    }

    template<typename T>
    static T systemDefaultEnum(const QString &name, const T &hardcoded = T())
    {
        return static_cast<T>(systemDefault(name, static_cast<int>(hardcoded)));
    }
};
