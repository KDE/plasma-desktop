/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xkb_rules.h"
#include "config-workspace.h"
#include "debug.h"

#include <KLocalizedString>

#include <QDir>
#include <QRegExp>
#include <QTextDocument> // for Qt::escape
#include <QXmlAttributes>

#include <QtConcurrent>

#include "x11_helper.h"

// for findXkbRuleFile
#include <QX11Info>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/XKBrules.h>
#include <fixx11h.h>

class RulesHandler : public QXmlDefaultHandler
{
public:
    RulesHandler(Rules *rules_, bool fromExtras_)
        : rules(rules_)
        , fromExtras(fromExtras_)
    {
    }

    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes) override;
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
    bool characters(const QString &str) override;

private:
    QStringList path;
    Rules *rules;
    const bool fromExtras;
};

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
#ifdef __GNUC__
#if __GNUC__ == 4 && (__GNUC_MINOR__ == 8 && __GNUC_PATCHLEVEL__ < 3) || (__GNUC_MINOR__ == 7 && __GNUC_PATCHLEVEL__ < 4)
#warning Compiling with a workaround for GCC < 4.8.3 || GCC < 4.7.4 https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58800
    Q_FOREACH (T *x, list) {
        ConfigItem *y = static_cast<ConfigItem *>(x);
        if (y->name.isEmpty()) {
            list.removeAll(x);
        }
    }
#else
    QtConcurrent::blockingFilter(list, notEmpty);
#endif
#endif
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
        QRegExp regex(QStringLiteral("\\.xml$"));
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

    RulesHandler rulesHandler(rules, fromExtras);

    QXmlSimpleReader reader;
    reader.setContentHandler(&rulesHandler);
    reader.setErrorHandler(&rulesHandler);

    QXmlInputSource xmlInputSource(&file);

    qCDebug(KCM_KEYBOARD) << "Parsing xkb rules from" << file.fileName();

    if (!reader.parse(xmlInputSource)) {
        qCCritical(KCM_KEYBOARD) << "Failed to parse the rules file" << file.fileName();
        return nullptr;
    }

    postProcess(rules);

    return rules;
}

bool RulesHandler::startElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
    path << QString(qName);

    QString strPath = path.join(QLatin1String("/"));
    if (strPath.endsWith(QLatin1String("layoutList/layout/configItem"))) {
        rules->layoutInfos << new LayoutInfo(fromExtras);
    } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant"))) {
        rules->layoutInfos.last()->variantInfos << new VariantInfo(fromExtras);
    } else if (strPath.endsWith(QLatin1String("modelList/model"))) {
        rules->modelInfos << new ModelInfo();
    } else if (strPath.endsWith(QLatin1String("optionList/group"))) {
        rules->optionGroupInfos << new OptionGroupInfo();
        rules->optionGroupInfos.last()->exclusive = (attributes.value(QStringLiteral("allowMultipleSelection")) != QLatin1String("true"));
    } else if (strPath.endsWith(QLatin1String("optionList/group/option"))) {
        rules->optionGroupInfos.last()->optionInfos << new OptionInfo();
    } else if (strPath == ("xkbConfigRegistry") && !attributes.value(QStringLiteral("version")).isEmpty()) {
        rules->version = attributes.value(QStringLiteral("version"));
        qCDebug(KCM_KEYBOARD) << "xkbConfigRegistry version" << rules->version;
    }
    return true;
}

bool RulesHandler::endElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & /*qName*/)
{
    path.removeLast();
    return true;
}

bool RulesHandler::characters(const QString &str)
{
    if (!str.trimmed().isEmpty()) {
        QString strPath = path.join(QLatin1String("/"));
        if (strPath.endsWith(QLatin1String("layoutList/layout/configItem/name"))) {
            if (rules->layoutInfos.last() != nullptr) {
                rules->layoutInfos.last()->name = str.trimmed();
            }
        } else if (strPath.endsWith(QLatin1String("layoutList/layout/configItem/description"))) {
            rules->layoutInfos.last()->description = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("layoutList/layout/configItem/languageList/iso639Id"))) {
            rules->layoutInfos.last()->languages << str.trimmed();
        } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant/configItem/name"))) {
            rules->layoutInfos.last()->variantInfos.last()->name = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant/configItem/description"))) {
            rules->layoutInfos.last()->variantInfos.last()->description = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("layoutList/layout/variantList/variant/configItem/languageList/iso639Id"))) {
            rules->layoutInfos.last()->variantInfos.last()->languages << str.trimmed();
        } else if (strPath.endsWith(QLatin1String("modelList/model/configItem/name"))) {
            rules->modelInfos.last()->name = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("modelList/model/configItem/description"))) {
            rules->modelInfos.last()->description = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("modelList/model/configItem/vendor"))) {
            rules->modelInfos.last()->vendor = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("optionList/group/configItem/name"))) {
            rules->optionGroupInfos.last()->name = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("optionList/group/configItem/description"))) {
            rules->optionGroupInfos.last()->description = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("optionList/group/option/configItem/name"))) {
            rules->optionGroupInfos.last()->optionInfos.last()->name = str.trimmed();
        } else if (strPath.endsWith(QLatin1String("optionList/group/option/configItem/description"))) {
            rules->optionGroupInfos.last()->optionInfos.last()->description = str.trimmed();
        }
    }
    return true;
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
