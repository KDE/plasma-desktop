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

#ifndef CUSTOMCONFIGDIALOGMANAGER_H
#define CUSTOMCONFIGDIALOGMANAGER_H

#include <QMap>
#include <QVariantHash>

#include <KConfigDialogManager>

class CustomConfigDialogManager : public KConfigDialogManager
{
    Q_OBJECT
public:
    CustomConfigDialogManager(QWidget *parent,
                              KCoreConfigSkeleton *config,
                              const QStringList &supported);
    ~CustomConfigDialogManager();

    QVariantHash currentWidgetProperties() const;
    void setWidgetProperties(const QVariantHash &);
    bool compareWidgetProperties(const QVariantHash &) const;
    bool hasChangedFuzzy() const;

private:
    QVariant fixup(QWidget *widget, QVariant value) const;

    QMap<QString, QWidget *> m_widgets;
    KCoreConfigSkeleton *m_config;
};

#endif // CUSTOMCONFIGDIALOGMANAGER_H
