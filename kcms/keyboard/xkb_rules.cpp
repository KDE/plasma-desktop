/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xkb_rules.h"
#include "config-workspace.h"
#include "debug.h"

#include <KLocalizedString>

#include <QtConcurrentFilter>

#include <xkbcommon/xkbregistry.h>

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

template<typename T>
static QString translate_description(T item)
{
    return item.description.isEmpty() ? item.name : translate_xml_item(item.description);
}

template<typename T>
static bool notEmpty(const T &item)
{
    return !item.name.isEmpty();
}

template<class T>
void removeEmptyItems(QList<T> &list)
{
    QMutableListIterator<T> it(list);

    while (it.hasNext()) {
        T t = it.next();
        if (!notEmpty(t)) {
            it.remove();
        }
    }
}

static void postProcess(Rules &rules)
{
    // TODO remove elements with empty names to safeguard us
    removeEmptyItems(rules.layoutInfos);
    removeEmptyItems(rules.modelInfos);
    removeEmptyItems(rules.optionGroupInfos);

    //	bindtextdomain("xkeyboard-config", LOCALE_DIR);
    for (ModelInfo &modelInfo : rules.modelInfos) {
        modelInfo.vendor = translate_xml_item(modelInfo.vendor);
        modelInfo.description = translate_description(modelInfo);
    }

    for (LayoutInfo &layoutInfo : rules.layoutInfos) {
        layoutInfo.description = translate_description(layoutInfo);

        removeEmptyItems(layoutInfo.variantInfos);
        for (VariantInfo &variantInfo : layoutInfo.variantInfos) {
            variantInfo.description = translate_description(variantInfo);
        }
    }
    for (OptionGroupInfo &optionGroupInfo : rules.optionGroupInfos) {
        optionGroupInfo.description = translate_description(optionGroupInfo);

        removeEmptyItems(optionGroupInfo.optionInfos);
        for (OptionInfo &optionInfo : optionGroupInfo.optionInfos) {
            optionInfo.description = translate_description(optionInfo);
        }
    }
}

const char Rules::XKB_OPTION_GROUP_SEPARATOR = ':';

static void rxkbLogHandler(rxkb_context *context, rxkb_log_level priority, const char *format, va_list args)
{
    Q_UNUSED(context)
    char buf[1024];
    int length = std::vsnprintf(buf, 1023, format, args);
    while (length > 0 && std::isspace(buf[length - 1])) {
        --length;
    }
    if (length <= 0) {
        return;
    }
    switch (priority) {
    case RXKB_LOG_LEVEL_DEBUG:
        qCDebug(KCM_KEYBOARD(), "XKB: %.*s", length, buf);
        break;
    case RXKB_LOG_LEVEL_INFO:
        qCInfo(KCM_KEYBOARD(), "XKB: %.*s", length, buf);
        break;
    case RXKB_LOG_LEVEL_WARNING:
        qCWarning(KCM_KEYBOARD(), "XKB: %.*s", length, buf);
        break;
    case RXKB_LOG_LEVEL_ERROR:
    case RXKB_LOG_LEVEL_CRITICAL:
    default:
        qCCritical(KCM_KEYBOARD(), "XKB: %.*s", length, buf);
        break;
    }
}

Rules &Rules::self()
{
    static Rules instance = readRules();
    return instance;
}

Rules Rules::readRules()
{
    rxkb_context *context = rxkb_context_new(RXKB_CONTEXT_LOAD_EXOTIC_RULES);
    if (!context) {
        qCDebug(KCM_KEYBOARD) << "Could not create xkb-registry context";
        return {};
    }
    rxkb_context_set_log_level(context, RXKB_LOG_LEVEL_DEBUG);
    rxkb_context_set_log_fn(context, &rxkbLogHandler);

    if (!rxkb_context_parse_default_ruleset(context)) {
        rxkb_context_unref(context);
        qCDebug(KCM_KEYBOARD) << "Could not parse xkb rules";
        return {};
    }
    Rules rules;

    rxkb_model *m = rxkb_model_first(context);
    while (m != nullptr) {
        rules.modelInfos << ModelInfo(rxkb_model_get_name(m), rxkb_model_get_description(m), rxkb_model_get_vendor(m));
        m = rxkb_model_next(m);
    }

    rxkb_layout *l = rxkb_layout_first(context);
    rxkb_iso639_code *iso639;
    int layoutIndex = -1;
    while (l != nullptr) {
        QStringList languages;
        iso639 = rxkb_layout_get_iso639_first(l);
        while (iso639 != nullptr) {
            languages << QString::fromUtf8(rxkb_iso639_code_get_code(iso639));
            iso639 = rxkb_iso639_code_next(iso639);
        }

        const char *variant = rxkb_layout_get_variant(l);
        if (variant == nullptr) {
            LayoutInfo layout(rxkb_layout_get_name(l), rxkb_layout_get_description(l), rxkb_layout_get_popularity(l) == RXKB_POPULARITY_EXOTIC);
            layout.languages = languages;
            layoutIndex = rules.layoutInfos.size();
            rules.layoutInfos << layout;
        } else if (layoutIndex != -1) {
            VariantInfo v(variant, rxkb_layout_get_description(l), rxkb_layout_get_popularity(l) == RXKB_POPULARITY_EXOTIC);
            v.languages = languages;
            rules.layoutInfos[layoutIndex].variantInfos << v;
        }
        l = rxkb_layout_next(l);
    }

    rxkb_option_group *g = rxkb_option_group_first(context);

    while (g != nullptr) {
        OptionGroupInfo group(rxkb_option_group_get_name(g), rxkb_option_group_get_description(g), !rxkb_option_group_allows_multiple(g));

        rxkb_option *o = rxkb_option_first(g);
        while (o != nullptr) {
            group.optionInfos << OptionInfo(rxkb_option_get_name(o), rxkb_option_get_description(o));
            o = rxkb_option_next(o);
        }
        rules.optionGroupInfos << group;
        g = rxkb_option_group_next(g);
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
    for (const VariantInfo &info : std::as_const(variantInfos)) {
        if (info.languages.contains(lang))
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
