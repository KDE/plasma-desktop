/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "customconfigdialogmanager.h"

#include <cmath>

#include <QGroupBox>
#include <QWidget>

#include <KComboBox>
#include <KConfigSkeleton>
#include <QDebug>

#include "customslider.h"

CustomConfigDialogManager::CustomConfigDialogManager(QWidget *parent, KCoreConfigSkeleton *conf, const QStringList &supported)
    : KConfigDialogManager(parent, conf)
    , m_config(conf)
{
    static const QString kcfgPrefix("kcfg_");

    const auto itemList = conf->items();
    for (KConfigSkeletonItem *i : itemList) {
        QString name(i->name());

        QWidget *child = parent->findChild<QWidget *>(kcfgPrefix + name);
        if (!child) {
            continue;
        }
        m_widgets[name] = child;

        /* FIXME: this should probably be less hackish */
        if (name == "Tapping" && !supported.contains("Tapping"))
            qobject_cast<QGroupBox *>(child)->setCheckable(false);
        else if (!supported.contains(name)) {
            child->setEnabled(false);
        }

        KCoreConfigSkeleton::ItemEnum *asEnum = dynamic_cast<KCoreConfigSkeleton::ItemEnum *>(i);
        if (!asEnum) {
            continue;
        }

        QStringList choiceList;
        const auto asEnumChoices = asEnum->choices();
        for (const auto &choice : asEnumChoices) {
            choiceList.append(!choice.label.isEmpty() ? choice.label : choice.name);
        }

        KComboBox *asComboBox = qobject_cast<KComboBox *>(child);
        if (asComboBox) {
            asComboBox->addItems(choiceList);
        }
    }
}

CustomConfigDialogManager::~CustomConfigDialogManager()
{
}

QVariantHash CustomConfigDialogManager::currentWidgetProperties() const
{
    QVariantHash r;
    for (QMap<QString, QWidget *>::ConstIterator i = m_widgets.begin(); i != m_widgets.end(); ++i) {
        r[i.key()] = property(i.value());
    }
    return r;
}

void CustomConfigDialogManager::setWidgetProperties(const QVariantHash &p)
{
    for (QVariantHash::ConstIterator i = p.begin(); i != p.end(); ++i) {
        QMap<QString, QWidget *>::ConstIterator j = m_widgets.constFind(i.key());
        if (j != m_widgets.constEnd()) {
            setProperty(j.value(), i.value());
        }
    }
}

static bool variantFuzzyCompare(const QVariant &a, const QVariant &b)
{
    if (a == b) {
        return true;
    }

    bool isDouble_a = false, isDouble_b = false;
    float d_a = static_cast<float>(a.toDouble(&isDouble_a)), d_b = static_cast<float>(b.toDouble(&isDouble_b));
    if (!isDouble_a || !isDouble_b) {
        return false;
    }

    return (qFuzzyIsNull(d_a) && qFuzzyIsNull(d_b)) || qFuzzyCompare(d_a, d_b);
}

QVariant CustomConfigDialogManager::fixup(QWidget *widget, QVariant v) const
{
    bool isDouble = false;
    double value = v.toDouble(&isDouble);
    if (!isDouble) {
        return v;
    }

    QVariant decimals(widget->property("decimals"));
    if (decimals.type() != QVariant::Int) {
        CustomSlider *asSlider = qobject_cast<CustomSlider *>(widget);
        if (asSlider) {
            return asSlider->fixup(value);
        }
        return value;
    }

    double k = std::pow(10.0, decimals.toInt());
    return std::floor(value * k + 0.5) / k; // round
}

bool CustomConfigDialogManager::compareWidgetProperties(const QVariantHash &p) const
{
    bool result = true;
    for (QVariantHash::ConstIterator i = p.begin(); i != p.end(); ++i) {
        QMap<QString, QWidget *>::ConstIterator j = m_widgets.find(i.key());
        if (j == m_widgets.end()) {
            continue;
        }

        QWidget *widget = j.value();
        QVariant widgetValue(fixup(widget, property(widget)));
        QVariant fixed(fixup(widget, i.value()));
        if (!variantFuzzyCompare(widgetValue, fixed)) {
            result = false;
            qDebug() << "Config mismatch:" << widget->objectName() << widgetValue << fixed;
        }
    }
    return result;
}

bool CustomConfigDialogManager::hasChangedFuzzy() const
{
    for (QMap<QString, QWidget *>::ConstIterator i = m_widgets.begin(); i != m_widgets.end(); ++i) {
        KConfigSkeletonItem *config = m_config->findItem(i.key());
        QWidget *widget = i.value();
        QVariant widgetValue(fixup(widget, property(widget)));
        QVariant configValue(fixup(widget, config->property()));
        if (!variantFuzzyCompare(widgetValue, configValue)) {
            return true;
        }
    }
    return false;
}
