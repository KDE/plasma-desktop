/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xkb_rules.h"
#include "config-workspace.h"
#include "debug.h"

#include <KLocalizedString>

#include <QDir>
#include <QRegularExpression>
#include <QTextDocument> // for Qt::escape
#include <QXmlStreamReader>

#include <QtConcurrent>

#include "x11_helper.h"

// for findXkbRuleFile
#include <QtGui/private/qtx11extras_p.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/XKBrules.h>
#include <fixx11h.h>

static QString translate_xml_item(const QString &itemText)
{
    if (itemText.isEmpty()) { // i18n warns on empty input strings
        return itemText;
    }
    // messages are already extracted from the source XML files by xkb
    // the characters '<' and '>' (but not '"') are HTML-escaped in the xkeyboard-config translation files, so we need to convert them before/after looking up
    // the translation note that we cannot use QString::toHtmlEscaped() here because that would convert '"' as well
    QString msgid(itemText);
    return i18nd("xkeyboard-config", msgid.replace(QLatin1String("<"), QLatin1String("&lt;")).replace(QLatin1String(">"), QLatin1String("&gt;")).toUtf8())
        .replace(QLatin1String("&lt;"), QLatin1String("<"))
        .replace(QLatin1String("&gt;"), QLatin1String(">"));
}

static QString translate_description(ConfigItem *item)
{
    return item->description.isEmpty() ? item->name : translate_xml_item(item->description);
}

static bool notEmpty(const ConfigItem *item)
{
    return !item->name.isEmpty();
}

template<class T>
void removeEmptyItems(QList<T *> &list)
{
    QtConcurrent::blockingFilter(list, notEmpty);
}

static void postProcess(Rules *rules)
{
    // TODO remove elements with empty names to safeguard us
    removeEmptyItems(rules->layoutInfos);
    removeEmptyItems(rules->modelInfos);
    removeEmptyItems(rules->optionGroupInfos);

    //	bindtextdomain("xkeyboard-config", LOCALE_DIR);
    for (ModelInfo *modelInfo : qAsConst(rules->modelInfos)) {
        modelInfo->vendor = translate_xml_item(modelInfo->vendor);
        modelInfo->description = translate_description(modelInfo);
    }

    for (LayoutInfo *layoutInfo : qAsConst(rules->layoutInfos)) {
        layoutInfo->description = translate_description(layoutInfo);

        removeEmptyItems(layoutInfo->variantInfos);
        for (VariantInfo *variantInfo : qAsConst(layoutInfo->variantInfos)) {
            variantInfo->description = translate_description(variantInfo);
        }
    }
    for (OptionGroupInfo *optionGroupInfo : qAsConst(rules->optionGroupInfos)) {
        optionGroupInfo->description = translate_description(optionGroupInfo);

        removeEmptyItems(optionGroupInfo->optionInfos);
        for (OptionInfo *optionInfo : qAsConst(optionGroupInfo->optionInfos)) {
            optionInfo->description = translate_description(optionInfo);
        }
    }
}

Rules::Rules()
    : version(QStringLiteral("1.0"))
{
}

QString Rules::getRulesName()
{
    if (!QX11Info::isPlatformX11()) {
        return QString();
    }
    XkbRF_VarDefsRec vd;
    char *tmp = nullptr;

    if (XkbRF_GetNamesProp(QX11Info::display(), &tmp, &vd) && tmp != nullptr) {
        // 			qCDebug(KCM_KEYBOARD) << "namesprop" << tmp ;
        const QString name(tmp);
        XFree(tmp);
        return name;
    }

    return {};
}

QString Rules::findXkbDir()
{
    return QStringLiteral(XKBDIR);
}

static QString findXkbRulesFile()
{
    QString rulesFile;
    QString rulesName = Rules::getRulesName();

    const QString xkbDir = Rules::findXkbDir();
    if (!rulesName.isNull()) {
        rulesFile = QStringLiteral("%1/rules/%2.xml").arg(xkbDir, rulesName);
    } else {
        // default to evdev
        rulesFile = QStringLiteral("%1/rules/evdev.xml").arg(xkbDir);
    }

    return rulesFile;
}

static void mergeRules(Rules *rules, Rules *extraRules)
{
    rules->modelInfos.append(extraRules->modelInfos);
    rules->optionGroupInfos.append(extraRules->optionGroupInfos); // need to iterate and merge?

    QList<LayoutInfo *> layoutsToAdd;
    for (LayoutInfo *extraLayoutInfo : qAsConst(extraRules->layoutInfos)) {
        LayoutInfo *layoutInfo = findByName(rules->layoutInfos, extraLayoutInfo->name);
        if (layoutInfo != nullptr) {
            layoutInfo->variantInfos.append(extraLayoutInfo->variantInfos);
            layoutInfo->languages.append(extraLayoutInfo->languages);
        } else {
            layoutsToAdd.append(extraLayoutInfo);
        }
    }
    rules->layoutInfos.append(layoutsToAdd);
    qCDebug(KCM_KEYBOARD) << "Merged from extra rules:" << extraRules->layoutInfos.size() << "layouts," << extraRules->modelInfos.size() << "models,"
                          << extraRules->optionGroupInfos.size() << "option groups";

    // base rules now own the objects - remove them from extra rules so that it does not try to delete them
    extraRules->layoutInfos.clear();
    extraRules->modelInfos.clear();
    extraRules->optionGroupInfos.clear();
}

const char Rules::XKB_OPTION_GROUP_SEPARATOR = ':';

Rules *Rules::readRules(ExtrasFlag extrasFlag)
{
    Rules *rules = new Rules();
    QString rulesFile = findXkbRulesFile();
    if (!readRules(rules, rulesFile, false)) {
        delete rules;
        return nullptr;
    }
    if (extrasFlag == Rules::READ_EXTRAS) {
        QRegularExpression regex(QStringLiteral("\\.xml$"));
        Rules *rulesExtra = new Rules();
        QString extraRulesFile = rulesFile.replace(regex, QStringLiteral(".extras.xml"));
        if (readRules(rulesExtra, extraRulesFile, true)) { // not fatal if it fails
            mergeRules(rules, rulesExtra);
        }
        delete rulesExtra;
    }
    return rules;
}

Rules *Rules::readRules(Rules *rules, const QString &filename, bool fromExtras)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCCritical(KCM_KEYBOARD) << "Cannot open the rules file" << file.fileName();
        return nullptr;
    }

    QStringList path;
    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            path << reader.name().toString();
            QString strPath = path.join(QLatin1String("/"));

            if (strPath.endsWith(QLatin1String("layoutList/layout/configItem"))) {
                rules->layoutInfos << new LayoutInfo(fromExtras);
            } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant"))) {
                rules->layoutInfos.last()->variantInfos << new VariantInfo(fromExtras);
            } else if (strPath.endsWith(QLatin1String("modelList/model"))) {
                rules->modelInfos << new ModelInfo();
            } else if (strPath.endsWith(QLatin1String("optionList/group"))) {
                rules->optionGroupInfos << new OptionGroupInfo();
                rules->optionGroupInfos.last()->exclusive = (reader.attributes().value(QStringLiteral("allowMultipleSelection")) != QLatin1String("true"));
            } else if (strPath.endsWith(QLatin1String("optionList/group/option"))) {
                rules->optionGroupInfos.last()->optionInfos << new OptionInfo();
            } else if (strPath == ("xkbConfigRegistry") && !reader.attributes().value(QStringLiteral("version")).isEmpty()) {
                rules->version = reader.attributes().value(QStringLiteral("version")).toString();
                qCDebug(KCM_KEYBOARD) << "xkbConfigRegistry version" << rules->version;
            }

            if (strPath.endsWith(QLatin1String("layoutList/layout/configItem/name"))) {
                if (rules->layoutInfos.last() != nullptr) {
                    rules->layoutInfos.last()->name = reader.readElementText().trimmed();
                }
            } else if (strPath.endsWith(QLatin1String("layoutList/layout/configItem/description"))) {
                rules->layoutInfos.last()->description = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("layoutList/layout/configItem/languageList/iso639Id"))) {
                rules->layoutInfos.last()->languages << reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant/configItem/name"))) {
                rules->layoutInfos.last()->variantInfos.last()->name = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant/configItem/description"))) {
                rules->layoutInfos.last()->variantInfos.last()->description = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant/configItem/languageList/iso639Id"))) {
                rules->layoutInfos.last()->variantInfos.last()->languages << reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("modelList/model/configItem/name"))) {
                rules->modelInfos.last()->name = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("modelList/model/configItem/description"))) {
                rules->modelInfos.last()->description = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("modelList/model/configItem/vendor"))) {
                rules->modelInfos.last()->vendor = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("optionList/group/configItem/name"))) {
                rules->optionGroupInfos.last()->name = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("optionList/group/configItem/description"))) {
                rules->optionGroupInfos.last()->description = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("optionList/group/option/configItem/name"))) {
                rules->optionGroupInfos.last()->optionInfos.last()->name = reader.readElementText().trimmed();
            } else if (strPath.endsWith(QLatin1String("optionList/group/option/configItem/description"))) {
                rules->optionGroupInfos.last()->optionInfos.last()->description = reader.readElementText().trimmed();
            }
        }
        // don't use token here, readElementText() above can have moved us forward meanwhile
        if (reader.tokenType() == QXmlStreamReader::EndElement) {
            path.removeLast();
        }
    }

    qCDebug(KCM_KEYBOARD) << "Parsing xkb rules from" << file.fileName();

    if (reader.hasError()) {
        qCCritical(KCM_KEYBOARD) << "Failed to parse the rules file" << file.fileName();
        return nullptr;
    }

    postProcess(rules);

    return rules;
}

bool LayoutInfo::isLanguageSupportedByLayout(const QString &lang) const
{
    return languages.contains(lang) || isLanguageSupportedByVariants(lang);
}

bool LayoutInfo::isLanguageSupportedByVariants(const QString &lang) const
{
    for (const VariantInfo *info : qAsConst(variantInfos)) {
        if (info->languages.contains(lang))
            return true;
    }
    return false;
}

bool LayoutInfo::isLanguageSupportedByDefaultVariant(const QString &lang) const
{
    if (languages.contains(lang))
        return true;

    if (languages.empty() && isLanguageSupportedByVariants(lang))
        return true;

    return false;
}

bool LayoutInfo::isLanguageSupportedByVariant(const VariantInfo *variantInfo, const QString &lang) const
{
    if (variantInfo->languages.contains(lang))
        return true;

    // if variant has no languages try to "inherit" them from layout
    if (variantInfo->languages.empty() && languages.contains(lang))
        return true;

    return false;
}
