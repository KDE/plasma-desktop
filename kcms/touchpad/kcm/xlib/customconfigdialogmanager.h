/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    CustomConfigDialogManager(QWidget *parent, KCoreConfigSkeleton *config, const QStringList &supported);
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
