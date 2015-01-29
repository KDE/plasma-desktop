/*  This file is part of the KDE's Plasma workspace
 *  Copyright 2014 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KCMTRANSLATIONS_H
#define KCMTRANSLATIONS_H

#include <QMap>

#include <KCModule>
#include <KSharedConfig>
#include <KConfigGroup>

class QListWidgetItem;
class KMessageWidget;

namespace Ui
{
class KCMTranslationsWidget;
}

/**
 * @short A KCM to configure KDE Gui Translations
 *
 * This module is for changing the User's Gui Translations settings.
 */

class KCMTranslations : public KCModule
{
    Q_OBJECT

public:
    KCMTranslations(QWidget *parent, const QVariantList &);
    virtual ~KCMTranslations();

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual QString quickHelp() const;

private Q_SLOTS:

    void changedTranslationsAvailable(QListWidgetItem *item);
    void changedTranslationsSelected(QListWidgetItem *item);

    void installTranslations();

private:

    void loadTranslations();
    void changedTranslations();

    void initWidgets();
    void initTranslations();
    void initTranslationsInstall();

    // The list of translations currently set in the KCM
    QStringList m_kcmTranslations;
    // The currently saved list of user translations, used to check if value changed
    QString m_configTranslations;
    // The currently installed translations, used to check if users translations are valid
    QStringList m_installedTranslations;

    KConfigGroup m_config;

    Ui::KCMTranslationsWidget *m_ui;
    KMessageWidget *m_messageWidget;
};

#endif //KCMTRANSLATIONS_H
