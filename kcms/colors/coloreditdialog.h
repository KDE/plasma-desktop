/* ColorEdit widget for KDE Display color scheme setup module
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

#ifndef __COLOREDIT_H__
#define __COLOREDIT_H__

#include <KColorScheme>
#include <KSharedConfig>

#include <QFrame>
#include <QPalette>
#include <QDialog>

#include "ui_coloredit.h"

#include "colorscm.h"

class ColorEditDialog : public QDialog, public Ui::ColorEdit
{
    Q_OBJECT

public:
    ColorEditDialog(QWidget *parent, KSharedConfigPtr config, const QString &schemeName);

Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:


    /** slot called when any varies button is clicked */
    void variesClicked();

    /** slot called when color on a KColorButton changes */
    void colorChanged( const QColor &newColor );


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

    /** slot called when the upload scheme button is clicked */
    void on_schemeKnsUploadButton_clicked();

    /** slot called when the save button is clicked */
    void on_schemeSaveButton_clicked();

    /** null slot to emit changed(true) */
    void emitChanged();

    /** set the colortable color buttons up according to the current colorset */
    void updateColorTable();

    /** update m_colorSchemes contents from the values in m_config */
    void updateColorSchemes();

    /** update the effects page from the values in m_config */
    void updateEffectsPage();

    /** update all preview panes from the values in m_config */
    void updatePreviews();

    /** setup the colortable with its buttons and labels */
    void setupColorTable();

private:

    /** copy color entries from color schemes into m_config */
    void updateFromColorSchemes();

    /** copy effects page entries from controls into m_config */
    void updateFromEffectsPage();

    void setCommonForeground(KColorScheme::ForegroundRole role,
                             int stackIndex,
                             int buttonIndex);
    void setCommonDecoration(KColorScheme::DecorationRole role,
                             int stackIndex,
                             int buttonIndex);
    void changeColor(int row, const QColor &newColor);

    /** helper to create color entries */
    void createColorEntry(const QString &text,
                          const QString &key,
                          QList<KColorButton *> &list,
                          int index);

    /** get the groupKey for the given colorSet */
    static QString colorSetGroupKey(int colorSet);

    /** save the current scheme with the given name
     @param name name to save the scheme as
     */
    void saveScheme(const QString &name);

    QList<KColorButton *> m_backgroundButtons;
    QList<KColorButton *> m_foregroundButtons;
    QList<KColorButton *> m_decorationButtons;
    QList<KColorButton *> m_commonColorButtons;
    QList<KColorScheme> m_colorSchemes;
    QList<QStackedWidget *> m_stackedWidgets;

    QStringList m_colorKeys;

    WindecoColors m_wmColors;

    QString m_schemeName;
    KSharedConfigPtr m_config;
    bool m_disableUpdates;
    bool m_loadedSchemeHasUnsavedChanges;

};

#endif
