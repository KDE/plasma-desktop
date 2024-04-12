/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>

struct ConfigItem {
    QString name;
    QString description;

    ConfigItem(const char *name_, const char *description_)
        : name(QString::fromUtf8(name_))
        , description(QString::fromUtf8(description_))
    {
    }
};

template<class T>
inline T *findByName(QList<T *> list, QString name)
{
    for (T *info : std::as_const(list)) {
        if (info->name == name)
            return info;
    }
    return nullptr;
}

struct VariantInfo : public ConfigItem {
    QStringList languages;
    const bool fromExtras;

    VariantInfo(const char *name_, const char *description_, bool fromExtras_)
        : ConfigItem(name_, description_)
        , fromExtras(fromExtras_)
    {
    }
};

struct LayoutInfo : public ConfigItem {
    QList<VariantInfo *> variantInfos;
    QStringList languages;
    const bool fromExtras;

    LayoutInfo(const char *name_, const char *description_, bool fromExtras_)
        : ConfigItem(name_, description_)
        , fromExtras(fromExtras_)
    {
    }
    ~LayoutInfo()
    {
        qDeleteAll(variantInfos);
    }

    VariantInfo *getVariantInfo(const QString &variantName) const
    {
        return findByName(variantInfos, variantName);
    }

    bool isLanguageSupportedByLayout(const QString &lang) const;
    bool isLanguageSupportedByDefaultVariant(const QString &lang) const;
    bool isLanguageSupportedByVariants(const QString &lang) const;
    bool isLanguageSupportedByVariant(const VariantInfo *variantInfo, const QString &lang) const;
};

struct ModelInfo : public ConfigItem {
    QString vendor;
    ModelInfo(const char *name_, const char *description_, const char *vendor_)
        : ConfigItem(name_, description_)
        , vendor(QString::fromUtf8(vendor_))
    {
    }
};

struct OptionInfo : public ConfigItem {
    OptionInfo(const char *name_, const char *description_)
        : ConfigItem(name_, description_)
    {
    }
};

struct OptionGroupInfo : public ConfigItem {
    QList<OptionInfo *> optionInfos;
    bool exclusive;
    OptionGroupInfo(const char *name_, const char *description_, bool exclusive_)
        : ConfigItem(name_, description_)
        , exclusive(exclusive_)
    {
    }

    ~OptionGroupInfo()
    {
        qDeleteAll(optionInfos);
    }

    const OptionInfo *getOptionInfo(const QString &optionName) const
    {
        return findByName(optionInfos, optionName);
    }
};

struct Rules {
    enum ExtrasFlag { NO_EXTRAS, READ_EXTRAS };

    static const char XKB_OPTION_GROUP_SEPARATOR;

    QList<LayoutInfo *> layoutInfos;
    QList<ModelInfo *> modelInfos;
    QList<OptionGroupInfo *> optionGroupInfos;

    Rules() = default;

    ~Rules()
    {
        qDeleteAll(layoutInfos);
        qDeleteAll(modelInfos);
        qDeleteAll(optionGroupInfos);
    }

    const LayoutInfo *getLayoutInfo(const QString &layoutName) const
    {
        return findByName(layoutInfos, layoutName);
    }

    const OptionGroupInfo *getOptionGroupInfo(const QString &optionGroupName) const
    {
        return findByName(optionGroupInfos, optionGroupName);
    }

    static Rules *readRules(ExtrasFlag extrasFlag);
};
