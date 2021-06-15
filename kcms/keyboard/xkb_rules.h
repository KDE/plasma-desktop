/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XKB_RULES_H_
#define XKB_RULES_H_

#include <QList>
#include <QStringList>
#include <QXmlDefaultHandler>

#include <config-keyboard.h>

struct ConfigItem {
    QString name;
    QString description;
};

template<class T>
inline T *findByName(QList<T *> list, QString name)
{
    for (T *info : qAsConst(list)) {
        if (info->name == name)
            return info;
    }
    return nullptr;
}

struct VariantInfo : public ConfigItem {
    QList<QString> languages;
    const bool fromExtras;

    VariantInfo(bool fromExtras_)
        : fromExtras(fromExtras_)
    {
    }
};

struct LayoutInfo : public ConfigItem {
    QList<VariantInfo *> variantInfos;
    QList<QString> languages;
    const bool fromExtras;

    //	LayoutInfo() {}
    LayoutInfo(bool fromExtras_)
        : fromExtras(fromExtras_)
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
};

struct OptionInfo : public ConfigItem {
};

struct OptionGroupInfo : public ConfigItem {
    QList<OptionInfo *> optionInfos;
    bool exclusive;

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
    QString version;

    Rules();

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
    static Rules *readRules(Rules *rules, const QString &filename, bool fromExtras);
    static QString getRulesName();
    static QString findXkbDir();
};

#endif /* XKB_RULES_H_ */
