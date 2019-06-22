/*
 *  kcmformats.h
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */
#ifndef __kcmformats_h__
#define __kcmformats_h__

#include <KCModule>
#include <KConfigGroup>

#include <QHash>
#include <QIcon>

namespace Ui
{
class KCMFormatsWidget;
}
class QComboBox;

class KCMFormats : public KCModule
{
    Q_OBJECT

public:
    explicit KCMFormats(QWidget *parent = nullptr, const QVariantList &list = QVariantList());
    ~KCMFormats() override;

    void load() override;
    void save() override;
    void defaults() override;

private:
    void addLocaleToCombo(QComboBox *combo, const QLocale &locale);
    void initCombo(QComboBox *combo, const QList<QLocale> &allLocales);
    void connectCombo(QComboBox *combo);
    QList<QComboBox *> m_combos;

    QIcon loadFlagIcon(const QString &flagCode);
    QHash<QString, QIcon> m_cachedFlags;
    QIcon m_cachedUnknown;

    void readConfig();
    void writeConfig();

    void updateExample();
    void updateEnabled();

    Ui::KCMFormatsWidget *m_ui;
    KConfigGroup m_config;
};

#endif
