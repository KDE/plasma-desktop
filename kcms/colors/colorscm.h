/* KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>
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
#include <KColorScheme>

#include "ui_colorsettings.h"

class QStackedWidget;
class QListWidgetItem;

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

    /// load the settings from the config
    virtual void load();

    /// save the current settings
    virtual void save();

    /// sets the configuration to sensible default values.
    virtual void defaults();

private Q_SLOTS:

    /** set the colortable color buttons up according to the current colorset */
    void updateColorTable();

    /** slot called when color on a KColorButton changes */
    void colorChanged( const QColor &newColor );

    /** slot called when any varies button is clicked */
    void variesClicked();

    /** slot called when the schemeList selection changes */
    void loadScheme(QListWidgetItem *currentItem, QListWidgetItem *previousItem);

    /** reselect the previously selected scheme in schemeList without loading it */
    void selectPreviousSchemeAgain();

    /** slot called when the remove button is clicked*/
    void on_schemeRemoveButton_clicked();

    /** slot called when the save button is clicked */
    void on_schemeSaveButton_clicked();

    /** slot called when the import button is clicked */
    void on_schemeImportButton_clicked();

    /** slot called when the get new schemes button is clicked */
    void on_schemeKnsButton_clicked();
    
    /** slot called when the upload scheme button is clicked */
    void on_schemeKnsUploadButton_clicked();

    /** null slot to emit changed(true) */
    void emitChanged();

    // options slots
    void on_contrastSlider_valueChanged(int value);
    void on_shadeSortedColumn_stateChanged(int state);
    void on_inactiveSelectionEffect_stateChanged(int state);
    void on_useInactiveEffects_stateChanged(int state);

    // effects page slots
    void on_inactiveIntensityBox_currentIndexChanged(int index);
    void on_inactiveIntensitySlider_valueChanged(int value);
    void on_inactiveColorBox_currentIndexChanged(int index);
    void on_inactiveColorSlider_valueChanged(int value);
    void on_inactiveColorButton_changed(const QColor & color);
    void on_inactiveContrastBox_currentIndexChanged(int index);
    void on_inactiveContrastSlider_valueChanged(int value);

    void on_disabledIntensityBox_currentIndexChanged(int index);
    void on_disabledIntensitySlider_valueChanged(int value);
    void on_disabledColorBox_currentIndexChanged(int index);
    void on_disabledColorSlider_valueChanged(int value);
    void on_disabledColorButton_changed(const QColor & color);
    void on_disabledContrastBox_currentIndexChanged(int index);
    void on_disabledContrastSlider_valueChanged(int value);

private:
    class WindecoColors {
        public:
            enum Role {
                ActiveForeground = 0,
                ActiveBackground = 1,
                InactiveForeground = 2,
                InactiveBackground = 3,
                ActiveBlend = 4,
                InactiveBlend = 5
            };

            WindecoColors() {}
            WindecoColors(const KSharedConfigPtr&);
            virtual ~WindecoColors() {}

            void load(const KSharedConfigPtr&);
            QColor color(Role) const;
        private:
            QColor m_colors[6];
    };

    /** create a preview of a color scheme */
    static QPixmap createSchemePreviewIcon(const KSharedConfigPtr &config);

    /** load options from global */
    void loadOptions();

    /** load from global */
    void loadInternal(bool loadOptions);

    /** load a scheme from a config file at a given path */
    void loadScheme(KSharedConfigPtr config);

    /** populate the schemeList with color schemes found on the system */
    void populateSchemeList();

    /** update m_colorSchemes contents from the values in m_config */
    void updateColorSchemes();

    /** update the effects page from the values in m_config */
    void updateEffectsPage();

    /** update all preview panes from the values in m_config */
    void updatePreviews();

    /** setup the colortable with its buttons and labels */
    void setupColorTable();

    /** helper to create color entries */
    void createColorEntry(const QString &text,
                          const QString &key,
                          QList<KColorButton *> &list,
                          int index);

    /** copy color entries from color schemes into m_config */
    void updateFromColorSchemes();

    /** copy effects page entries from controls into m_config */
    void updateFromEffectsPage();

    /** copy options from controls into m_config */
    void updateFromOptions();

    void changeColor(int row, const QColor &newColor);

    /** get the groupKey for the given colorSet */
    static QString colorSetGroupKey(int colorSet);

    /** save the current scheme with the given name
     @param name name to save the scheme as
     */
    void saveScheme(const QString &name);

    void setCommonForeground(KColorScheme::ForegroundRole role,
                             int stackIndex,
                             int buttonIndex);
    void setCommonDecoration(KColorScheme::DecorationRole role,
                             int stackIndex,
                             int buttonIndex);

    QList<KColorButton *> m_backgroundButtons;
    QList<KColorButton *> m_foregroundButtons;
    QList<KColorButton *> m_decorationButtons;
    QList<KColorButton *> m_commonColorButtons;
    QList<QStackedWidget *> m_stackedWidgets;
    QStringList m_colorKeys;

    QList<KColorScheme> m_colorSchemes;
    WindecoColors m_wmColors;
    QString m_currentColorScheme;

    KSharedConfigPtr m_config;

    bool m_disableUpdates;
    bool m_loadedSchemeHasUnsavedChanges;
    // don't (re)load the scheme, only select it in schemeList
    bool m_dontLoadSelectedScheme;

    // the item previously selected in schemeList
    QListWidgetItem *m_previousSchemeItem;
};

#endif
