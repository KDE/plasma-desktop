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

namespace Ui
{
class KCMFormatsWidget;
}
class QComboBox;
class KMessageWidget;

class KCMFormats : public KCModule
{
    Q_OBJECT

public:
    explicit KCMFormats(QWidget *parent = 0, const QVariantList &list = QVariantList());
    ~KCMFormats();

    void load();
    void save();
    void defaults();

private:
    void addLocaleToCombo(QComboBox *combo, const QLocale &locale);
    void initCombo(QComboBox *combo, const QList<QLocale> &allLocales);
    void connectCombo(QComboBox *combo);
    QList<QComboBox *> m_combos;

    void readConfig();
    void writeConfig();

    void updateExample();
    void updateEnabled();

    Ui::KCMFormatsWidget *m_ui;
    KConfigGroup m_config;
};

#endif
