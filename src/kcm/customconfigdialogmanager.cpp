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

#include "customconfigdialogmanager.h"

#include <QWidget>

#include <KConfigSkeleton>
#include <KComboBox>

CustomConfigDialogManager::CustomConfigDialogManager(QWidget *parent,
                                                     KCoreConfigSkeleton *conf,
                                                     const QStringList &supported)
    : KConfigDialogManager(parent, conf)
{
    static const QString kcfgPrefix("kcfg_");

    Q_FOREACH(KConfigSkeletonItem *i, conf->items()) {
        QString name(i->name());

        QWidget *child = parent->findChild<QWidget*>(kcfgPrefix + name);
        if (!child) {
            continue;
        }
        m_widgets[name] = child;

        if (!supported.contains(name)) {
            child->setEnabled(false);
        }

        KCoreConfigSkeleton::ItemEnum *asEnum =
                dynamic_cast<KCoreConfigSkeleton::ItemEnum *>(i);
        if (!asEnum) {
            continue;
        }

        QStringList choiceList;
        Q_FOREACH(const KCoreConfigSkeleton::ItemEnum::Choice &c,
                  asEnum->choices())
        {
            if (c.label.isEmpty()) {
                choiceList.append(c.name);
            } else {
                choiceList.append(c.label);
            }
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
    for (QMap<QString, QWidget *>::ConstIterator i = m_widgets.begin();
         i != m_widgets.end(); i++)
    {
        r[i.key()] = property(i.value());
    }
    return r;
}

void CustomConfigDialogManager::setWidgetProperties(const QVariantHash &p)
{
    for (QVariantHash::ConstIterator i = p.begin(); i != p.end(); i++) {
        QMap<QString, QWidget *>::ConstIterator j = m_widgets.find(i.key());
        if (j != m_widgets.end()) {
            setProperty(j.value(), i.value());
        }
    }
}
