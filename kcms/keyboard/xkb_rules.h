/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>

template<class T>
inline std::optional<T> findByName(QList<T> list, QString name)
{
    for (T info : std::as_const(list)) {
        if (info.name == name)
            return info;
    }
    return std::nullopt;
}

struct VariantInfo {
    QString name;
    QString description;
    QStringList languages;
    bool fromExtras = false;

    VariantInfo(const char *name_, const char *description_, bool fromExtras_)
        : name(QString::fromUtf8(name_))
        , description(QString::fromUtf8(description_))
        , fromExtras(fromExtras_)
    {
    }
};

struct LayoutInfo {
    QString name;
    QString description;
    QList<VariantInfo> variantInfos;
    QStringList languages;
    bool fromExtras;

    LayoutInfo(const char *name_, const char *description_, bool fromExtras_)
        : name(QString::fromUtf8(name_))
        , description(QString::fromUtf8(description_))
        , fromExtras(fromExtras_)
    {
    }

    std::optional<VariantInfo> getVariantInfo(const QString &variantName) const
    {
        return findByName(variantInfos, variantName);
    }

    bool isLanguageSupportedByLayout(const QString &lang) const;
    bool isLanguageSupportedByDefaultVariant(const QString &lang) const;
    bool isLanguageSupportedByVariants(const QString &lang) const;
    bool isLanguageSupportedByVariant(const VariantInfo *variantInfo, const QString &lang) const;
};

struct ModelInfo {
    QString name;
    QString description;
    QString vendor;
    ModelInfo(const char *name_, const char *description_, const char *vendor_)
        : name(QString::fromUtf8(name_))
        , description(QString::fromUtf8(description_))
        , vendor(QString::fromUtf8(vendor_))
    {
    }
};

struct OptionInfo {
    QString name;
    QString description;
    OptionInfo(const char *name_, const char *description_)
        : name(QString::fromUtf8(name_))
        , description(QString::fromUtf8(description_))
    {
    }
};

struct OptionGroupInfo {
    QString name;
    QString description;
    QList<OptionInfo> optionInfos;
    bool exclusive;
    OptionGroupInfo(const char *name_, const char *description_, bool exclusive_)
        : name(QString::fromUtf8(name_))
        , description(QString::fromUtf8(description_))
        , exclusive(exclusive_)
    {
    }

    std::optional<OptionInfo> getOptionInfo(const QString &optionName) const
    {
        return findByName(optionInfos, optionName);
    }
};

struct Rules {
    enum ExtrasFlag { NO_EXTRAS, READ_EXTRAS };

    static const char XKB_OPTION_GROUP_SEPARATOR;

    QList<LayoutInfo> layoutInfos;
    QList<ModelInfo> modelInfos;
    QList<OptionGroupInfo> optionGroupInfos;

    std::optional<LayoutInfo> getLayoutInfo(const QString &layoutName) const
    {
        return findByName(layoutInfos, layoutName);
    }

    std::optional<OptionGroupInfo> getOptionGroupInfo(const QString &optionGroupName) const
    {
        return findByName(optionGroupInfos, optionGroupName);
    }

    static Rules *readRules(ExtrasFlag extrasFlag);
};
