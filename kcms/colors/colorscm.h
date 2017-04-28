/* KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __COLORSCM_H__
#define __COLORSCM_H__

#include <KCModule>

#include "ui_colorsettings.h"

class QStackedWidget;
class QListWidgetItem;

class KColorButton;


/**
 * The Desktop/Colors tab in kcontrol.
 */
class KColorCm : public KCModule, public Ui::colorSettings
{
    Q_OBJECT

public:
    KColorCm(QWidget *parent, const QVariantList &);
    ~KColorCm();

public Q_SLOTS:

    /**
     * load the settings from the config
     */
    virtual void load();

    /**
     * save the current settings
     */
    virtual void save();

    /**
     * sets the configuration to sensible default values.
     */
    virtual void defaults();

    /**
     * Update all keys of the Global config with the theme ones.
     */
    void updateConfig(KSharedConfigPtr config);

private Q_SLOTS:


    /**
     * slot called when the schemeList selection changes.
     *
     * It loads the scheme with loadScheme(KSharedConfigPtr), which updates the
     * config (updateConfig())
     */
    void loadScheme(QListWidgetItem *currentItem, QListWidgetItem *previousItem);

    /**
     * Reselect the previously selected scheme in schemeList without loading it
     */
    void selectPreviousSchemeAgain();

    /**
     * Slot called when the remove button is clicked
     */
    void on_schemeRemoveButton_clicked();

    /**
     * Slot called when the import button is clicked
     */
    void on_schemeImportButton_clicked();

    /**
     * slot called when the get new schemes button is clicked
     */
    void on_schemeKnsButton_clicked();

    /**
     * Slot called when the Edit button is clicked.
     *
     * It opens a dialog for the edition/creation.
     */
    void on_schemeEditButton_clicked();
private:

    /**
     * Create a preview icon of a color scheme
     */
    static QPixmap createSchemePreviewIcon(const KSharedConfigPtr &config);

    /**
     * Load from global.
     */
    void loadInternal();

    /**
     * load a scheme from a config file at a given path and updates the
     * config (updateConfig())
     */
    void loadScheme(KSharedConfigPtr config);

    /**
     * Populate the schemeList with color schemes found on the system
     */
    void populateSchemeList();


    QString m_currentColorScheme;

    // The global config
    KSharedConfigPtr m_config;

    // don't (re)load the scheme, only select it in schemeList
    bool m_dontLoadSelectedScheme;

    // the item previously selected in schemeList
    QListWidgetItem *m_previousSchemeItem;
};

#endif
