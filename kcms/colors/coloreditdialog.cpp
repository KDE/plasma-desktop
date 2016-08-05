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

#include "coloreditdialog.h"

#include <QColorDialog>
#include <QDebug>
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QUrl>

#include <KConfigGroup>
#include <KColorScheme>
#include <KMessageBox>

#include <KNS3/UploadDialog>

ColorEditDialog::ColorEditDialog(QWidget *parent, KSharedConfigPtr config,
                                 const QString &schemeName)
    : QDialog( parent )
    , m_disableUpdates(false)
    , m_config(config)
    , m_schemeName(schemeName)
{
    setupUi(this);
    schemeKnsUploadButton->setIcon( QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")) );
    connect(colorSet, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ColorEditDialog::updateColorTable);
    connect(applyToAlien, &QCheckBox::toggled, this, &ColorEditDialog::emitChanged);

    // only needs to be called once
    updatePreviews();
    setupColorTable();
}

void ColorEditDialog::emitChanged()
{
    emit changed(true);
}

void ColorEditDialog::updatePreviews()
{
    colorPreview->setPalette(m_config);
    inactivePreview->setPalette(m_config, QPalette::Inactive);
    disabledPreview->setPalette(m_config, QPalette::Disabled);
}

void ColorEditDialog::updateEffectsPage()
{
    m_disableUpdates = true;

    // NOTE: keep this in sync with kdelibs/kdeui/colors/kcolorscheme.cpp
    KConfigGroup groupI(m_config, "ColorEffects:Inactive");
    inactiveIntensityBox->setCurrentIndex(abs(groupI.readEntry("IntensityEffect", 0)));
    inactiveIntensitySlider->setValue(int(groupI.readEntry("IntensityAmount", 0.0) * 20.0) + 20);
    inactiveColorBox->setCurrentIndex(abs(groupI.readEntry("ColorEffect", 2)));
    if (inactiveColorBox->currentIndex() > 1)
    {
        inactiveColorSlider->setValue(int(groupI.readEntry("ColorAmount", 0.025) * 40.0));
    }
    else
    {
        inactiveColorSlider->setValue(int(groupI.readEntry("ColorAmount", 0.05) * 20.0) + 20);
    }
    inactiveColorButton->setColor(groupI.readEntry("Color", QColor(112, 111, 110)));
    inactiveContrastBox->setCurrentIndex(abs(groupI.readEntry("ContrastEffect", 2)));
    inactiveContrastSlider->setValue(int(groupI.readEntry("ContrastAmount", 0.1) * 20.0));

    // NOTE: keep this in sync with kdelibs/kdeui/colors/kcolorscheme.cpp
    KConfigGroup groupD(m_config, "ColorEffects:Disabled");
    disabledIntensityBox->setCurrentIndex(groupD.readEntry("IntensityEffect", 2));
    disabledIntensitySlider->setValue(int(groupD.readEntry("IntensityAmount", 0.1) * 20.0) + 20);
    disabledColorBox->setCurrentIndex(groupD.readEntry("ColorEffect", 0));
    if (disabledColorBox->currentIndex() > 1)
    {
        disabledColorSlider->setValue(int(groupD.readEntry("ColorAmount", 0.0) * 40.0));
    }
    else
    {
        disabledColorSlider->setValue(int(groupD.readEntry("ColorAmount", 0.0) * 20.0) + 20);
    }
    disabledColorButton->setColor(groupD.readEntry("Color", QColor(56, 56, 56)));
    disabledContrastBox->setCurrentIndex(groupD.readEntry("ContrastEffect", 1));
    disabledContrastSlider->setValue(int(groupD.readEntry("ContrastAmount", 0.65) * 20.0));

    m_disableUpdates = false;

    // enable/disable controls
    inactiveIntensitySlider->setDisabled(inactiveIntensityBox->currentIndex() == 0);
    disabledIntensitySlider->setDisabled(disabledIntensityBox->currentIndex() == 0);
    inactiveColorSlider->setDisabled(inactiveColorBox->currentIndex() == 0);
    disabledColorSlider->setDisabled(disabledColorBox->currentIndex() == 0);
    inactiveColorButton->setDisabled(inactiveColorBox->currentIndex() < 2);
    disabledColorButton->setDisabled(disabledColorBox->currentIndex() < 2);
    inactiveContrastSlider->setDisabled(inactiveContrastBox->currentIndex() == 0);
    disabledContrastSlider->setDisabled(disabledContrastBox->currentIndex() == 0);
}

void ColorEditDialog::updateColorTable()
{
    // subtract one here since the 0 item  is "Common Colors"
    const int currentSet = colorSet->currentIndex() - 1;

    if (currentSet == -1)
    {
        // common colors is selected
        stackColors->setCurrentIndex(0);
        // stackPreview->setCurrentIndex(0);

        KColorButton * button;
        foreach (button, m_commonColorButtons)
        {
            button->blockSignals(true);
        }

        m_commonColorButtons[0]->setColor(m_colorSchemes[KColorScheme::View].background(KColorScheme::NormalBackground).color());
        m_commonColorButtons[1]->setColor(m_colorSchemes[KColorScheme::View].foreground(KColorScheme::NormalText).color());
        m_commonColorButtons[2]->setColor(m_colorSchemes[KColorScheme::Window].background(KColorScheme::NormalBackground).color());
        m_commonColorButtons[3]->setColor(m_colorSchemes[KColorScheme::Window].foreground(KColorScheme::NormalText).color());
        m_commonColorButtons[4]->setColor(m_colorSchemes[KColorScheme::Button].background(KColorScheme::NormalBackground).color());
        m_commonColorButtons[5]->setColor(m_colorSchemes[KColorScheme::Button].foreground(KColorScheme::NormalText).color());
        m_commonColorButtons[6]->setColor(m_colorSchemes[KColorScheme::Selection].background(KColorScheme::NormalBackground).color());
        m_commonColorButtons[7]->setColor(m_colorSchemes[KColorScheme::Selection].foreground(KColorScheme::NormalText).color());
        m_commonColorButtons[8]->setColor(m_colorSchemes[KColorScheme::Selection].foreground(KColorScheme::InactiveText).color());

        setCommonForeground(KColorScheme::InactiveText, 0,  9);
        setCommonForeground(KColorScheme::ActiveText,   1, 10);
        setCommonForeground(KColorScheme::LinkText,     2, 11);
        setCommonForeground(KColorScheme::VisitedText,  3, 12);
        setCommonForeground(KColorScheme::NegativeText, 4, 13);
        setCommonForeground(KColorScheme::NeutralText,  5, 14);
        setCommonForeground(KColorScheme::PositiveText, 6, 15);

        setCommonDecoration(KColorScheme::FocusColor, 7, 16);
        setCommonDecoration(KColorScheme::HoverColor, 8, 17);

        m_commonColorButtons[18]->setColor(m_colorSchemes[KColorScheme::Tooltip].background(KColorScheme::NormalBackground).color());
        m_commonColorButtons[19]->setColor(m_colorSchemes[KColorScheme::Tooltip].foreground(KColorScheme::NormalText).color());

        m_commonColorButtons[20]->setColor(m_wmColors.color(WindecoColors::ActiveBackground));
        m_commonColorButtons[21]->setColor(m_wmColors.color(WindecoColors::ActiveForeground));
        m_commonColorButtons[22]->setColor(m_wmColors.color(WindecoColors::ActiveBlend));
        m_commonColorButtons[23]->setColor(m_wmColors.color(WindecoColors::InactiveBackground));
        m_commonColorButtons[24]->setColor(m_wmColors.color(WindecoColors::InactiveForeground));
        m_commonColorButtons[25]->setColor(m_wmColors.color(WindecoColors::InactiveBlend));

        foreach (button, m_commonColorButtons)
        {
            button->blockSignals(false);
        }
    }
    else
    {
        // a real color set is selected
        stackColors->setCurrentIndex(1);

        for (int i = KColorScheme::NormalBackground; i <= KColorScheme::AlternateBackground; ++i)
        {
            m_backgroundButtons[i]->blockSignals(true);
            m_backgroundButtons[i]->setColor(m_colorSchemes[currentSet].background(KColorScheme::BackgroundRole(i)).color());
            m_backgroundButtons[i]->blockSignals(false);
        }

        for (int i = KColorScheme::NormalText; i <= KColorScheme::PositiveText; ++i)
        {
            m_foregroundButtons[i]->blockSignals(true);
            m_foregroundButtons[i]->setColor(m_colorSchemes[currentSet].foreground(KColorScheme::ForegroundRole(i)).color());
            m_foregroundButtons[i]->blockSignals(false);
        }

        for (int i = KColorScheme::FocusColor; i <= KColorScheme::HoverColor; ++i)
        {
            m_decorationButtons[i]->blockSignals(true);
            m_decorationButtons[i]->setColor(m_colorSchemes[currentSet].decoration(KColorScheme::DecorationRole(i)).color());
            m_decorationButtons[i]->blockSignals(false);
        }
    }
}

void ColorEditDialog::colorChanged( const QColor &newColor )
{
    // find which button was changed
    const int row = sender()->objectName().toInt();
    changeColor(row, newColor);
}

void ColorEditDialog::changeColor(int row, const QColor &newColor)
{
    // update the m_colorSchemes for the selected colorSet
    const int currentSet = colorSet->currentIndex() - 1;

    if (currentSet == -1)
    {
        // common colors is selected
        switch (row)
        {
            case 0:
                // View Background button
                KConfigGroup(m_config, "Colors:View").writeEntry("BackgroundNormal", newColor);
                break;
            case 1:
                // View Text button
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundNormal", newColor);
                break;
            case 2:
                // Window Background Button
                KConfigGroup(m_config, "Colors:Window").writeEntry("BackgroundNormal", newColor);
                break;
            case 3:
                // Window Text Button
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundNormal", newColor);
                break;
            case 4:
                // Button Background button
                KConfigGroup(m_config, "Colors:Button").writeEntry("BackgroundNormal", newColor);
                break;
            case 5:
                // Button Text button
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundNormal", newColor);
                break;
            case 6:
                // Selection Background Button
                KConfigGroup(m_config, "Colors:Selection").writeEntry("BackgroundNormal", newColor);
                break;
            case 7:
                // Selection Text Button
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundNormal", newColor);
                break;
            case 8:
                // Selection Inactive Text Button
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundInactive", newColor);
                break;

            // buttons that could have varies in their place
            case 9:
                // Inactive Text Button (set all but Selection Inactive Text color)
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundInactive", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundInactive", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundInactive", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundInactive", newColor);
                break;
            case 10:
                // Active Text Button (set all active text colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundActive", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundActive", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundActive", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundActive", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundActive", newColor);
                break;
            case 11:
                // Link Text Button (set all link text colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundLink", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundLink", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundLink", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundLink", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundLink", newColor);
                break;
            case 12:
                // Visited Text Button (set all visited text colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundVisited", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundVisited", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundVisited", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundVisited", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundVisited", newColor);
                break;
            case 13:
                // Negative Text Button (set all negative text colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundNegative", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundNegative", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundNegative", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundNegative", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundNegative", newColor);
                break;
            case 14:
                // Neutral Text Button (set all neutral text colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundNeutral", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundNeutral", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundNeutral", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundNeutral", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundNeutral", newColor);
                break;
            case 15:
                // Positive Text Button (set all positive text colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("ForegroundPositive", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("ForegroundPositive", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("ForegroundPositive", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("ForegroundPositive", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundPositive", newColor);
                break;

            case 16:
                // Focus Decoration Button (set all focus decoration colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("DecorationFocus", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("DecorationFocus", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("DecorationFocus", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("DecorationFocus", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("DecorationFocus", newColor);
                break;
            case 17:
                // Hover Decoration Button (set all hover decoration colors)
                KConfigGroup(m_config, "Colors:View").writeEntry("DecorationHover", newColor);
                KConfigGroup(m_config, "Colors:Window").writeEntry("DecorationHover", newColor);
                KConfigGroup(m_config, "Colors:Selection").writeEntry("DecorationHover", newColor);
                KConfigGroup(m_config, "Colors:Button").writeEntry("DecorationHover", newColor);
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("DecorationHover", newColor);
                break;

            case 18:
                // Tooltip Background button
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("BackgroundNormal", newColor);
                break;
            case 19:
                // Tooltip Text button
                KConfigGroup(m_config, "Colors:Tooltip").writeEntry("ForegroundNormal", newColor);
                break;
            case 20:
                // Active Title Background
                KConfigGroup(m_config, "WM").writeEntry("activeBackground", newColor);
                break;
            case 21:
                // Active Title Text
                KConfigGroup(m_config, "WM").writeEntry("activeForeground", newColor);
                break;
            case 22:
                // Active Title Secondary
                KConfigGroup(m_config, "WM").writeEntry("activeBlend", newColor);
                break;
            case 23:
                // Inactive Title Background
                KConfigGroup(m_config, "WM").writeEntry("inactiveBackground", newColor);
                break;
            case 24:
                // Inactive Title Text
                KConfigGroup(m_config, "WM").writeEntry("inactiveForeground", newColor);
                break;
            case 25:
                // Inactive Title Secondary
                KConfigGroup(m_config, "WM").writeEntry("inactiveBlend", newColor);
                break;
        }
        m_commonColorButtons[row]->blockSignals(true);
        m_commonColorButtons[row]->setColor(newColor);
        m_commonColorButtons[row]->blockSignals(false);
    }
    else
    {
        QString group = colorSetGroupKey(currentSet);
        KConfigGroup(m_config, group).writeEntry(m_colorKeys[row], newColor);
    }

//FIXME    QIcon icon = createSchemePreviewIcon(m_config);
//FIXME    schemeList->item(0)->setIcon(icon);
    updateColorSchemes();
    updatePreviews();

    m_loadedSchemeHasUnsavedChanges = true;
    QString colorScheme = i18nc("Current color scheme", "Current");
    KConfigGroup group(m_config, "General");
    group.writeEntry("ColorScheme", colorScheme);
    schemeKnsUploadButton->setEnabled(false);

    emit changed(true);
}

QString ColorEditDialog::colorSetGroupKey(int colorSet)
{
    QString group;
    switch (colorSet) {
        case KColorScheme::Window:
            group = QStringLiteral("Colors:Window");
            break;
        case KColorScheme::Button:
            group = QStringLiteral("Colors:Button");
            break;
        case KColorScheme::Selection:
            group = QStringLiteral("Colors:Selection");
            break;
        case KColorScheme::Tooltip:
            group = QStringLiteral("Colors:Tooltip");
            break;
        default:
            group = QStringLiteral("Colors:View");
    }
    return group;
}

void ColorEditDialog::on_contrastSlider_valueChanged(int value)
{
    KConfigGroup group(m_config, "KDE");
    group.writeEntry("contrast", value);

    updatePreviews();

    emit changed(true);
}

void ColorEditDialog::on_shadeSortedColumn_stateChanged(int state)
{
    KConfigGroup group(m_config, "General");
    group.writeEntry("shadeSortColumn", bool(state != Qt::Unchecked));

    emit changed(true);
}

void ColorEditDialog::on_useInactiveEffects_stateChanged(int state)
{
    KConfigGroup group(m_config, "ColorEffects:Inactive");
    group.writeEntry("Enable", bool(state != Qt::Unchecked));

    m_disableUpdates = true;
    printf("re-init\n");
    inactiveSelectionEffect->setChecked(group.readEntry("ChangeSelectionColor", bool(state != Qt::Unchecked)));
    m_disableUpdates = false;

    if ((state != Qt::Unchecked) && tabWidget->count() < 5) {
        tabWidget->insertTab(4, pageInactive, i18n("Inactive"));
    } else if ((state == Qt::Unchecked) && tabWidget->count() > 4) {
        tabWidget->removeTab(4);
    }

    emit changed(true);
}

void ColorEditDialog::on_inactiveSelectionEffect_stateChanged(int state)
{
    if (m_disableUpdates)
    {
        // don't write the config as we are reading it!
        return;
    }

    KConfigGroup group(m_config, "ColorEffects:Inactive");
    group.writeEntry("ChangeSelectionColor", bool(state != Qt::Unchecked));

    emit changed(true);
}

void ColorEditDialog::on_schemeKnsUploadButton_clicked()
{
    if (m_loadedSchemeHasUnsavedChanges)
    {
        KMessageBox::ButtonCode reallyUpload = KMessageBox::questionYesNo(
            this, i18n("This colour scheme was not saved. Continue?"),
            i18n("Do you really want to upload?"));
        if (reallyUpload == KMessageBox::No) {
            return;
        }
    }

    // find path
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
        "color-schemes/" + m_schemeName + ".colors");
    if (path.isEmpty() ) // if the color scheme file wasn't found
    {
        qDebug() << "path for color scheme " << m_schemeName << " couldn't be found";
        return;
    }

    // upload
    KNS3::UploadDialog dialog(QStringLiteral("colorschemes.knsrc"), this);
    dialog.setUploadFile(QUrl::fromLocalFile(path) );
    dialog.exec();
}

void ColorEditDialog::on_schemeSaveButton_clicked()
{
    // prompt for the name to save as
    bool ok;
    QString name = QInputDialog::getText(this, i18n("Save Color Scheme"),
        i18n("&Enter a name for the color scheme:"), QLineEdit::Normal, m_schemeName, &ok);
    if (ok)
    {
        saveScheme(name);
    }
}

void ColorEditDialog::saveScheme(const QString &name)
{
    QString filename = name;
    filename.remove('\''); // So Foo's does not become FooS
    QRegExp fixer(QStringLiteral("[\\W,.-]+(.?)"));
    int offset;
    while ((offset = fixer.indexIn(filename)) >= 0)
        filename.replace(offset, fixer.matchedLength(), fixer.cap(1).toUpper());
    filename.replace(0, 1, filename.at(0).toUpper());

    // check if that name is already in the list
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
        "color-schemes/" + filename + ".colors");

    QFile file(path);
    const int permissions = file.permissions();
    const bool canWrite = (permissions & QFile::WriteUser);
    // or if we can overwrite it if it exists
    if (path.isEmpty() || !file.exists() || canWrite)
    {
        if(canWrite){
            int ret = KMessageBox::questionYesNo(this,
                i18n("A color scheme with that name already exists.\nDo you want to overwrite it?"),
                i18n("Save Color Scheme"),
                KStandardGuiItem::overwrite(),
                KStandardGuiItem::cancel());
            //on don't overwrite, select the already existing name.
            /*FIXME if(ret == KMessageBox::No){
                QList<QListWidgetItem*> foundItems = schemeList->findItems(name, Qt::MatchExactly);
                if (foundItems.size() == 1)
                    schemeList->setCurrentRow(schemeList->row(foundItems[0]));
                return;
            }*/
        }

        // go ahead and save it
        QString newpath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                + "/color-schemes/";
        QDir dir;
        dir.mkpath(newpath);
        newpath += filename + ".colors";
        KSharedConfigPtr temp = m_config;
        m_config = KSharedConfig::openConfig(newpath);
        // then copy current colors into new config
        updateFromColorSchemes();
        updateFromEffectsPage();
        KConfigGroup group(m_config, "General");
        group.writeEntry("Name", name);
        // sync it
        m_config->sync();

        m_loadedSchemeHasUnsavedChanges = false;

/*FIXME        QList<QListWidgetItem*> foundItems = schemeList->findItems(name, Qt::MatchExactly);
//FIXME        QIcon icon = createSchemePreviewIcon(m_config);
        if (foundItems.size() < 1)
        {
            // add it to the list since it's not in there already
            populateSchemeList();

            // then select the new item
            schemeList->setCurrentItem(schemeList->findItems(name, Qt::MatchExactly).at(0));
        }
        else
        {
            // update the icon of the one that's in the list
            foundItems[0]->setIcon(icon);
            schemeList->setCurrentRow(schemeList->row(foundItems[0]));
        }*/

        // set m_config back to the system one
        m_config = temp;

        // store colorscheme name in global settings
        group = KConfigGroup(m_config, "General");
        group.writeEntry("ColorScheme", name);

        emit changed(true);
    }
    else if (!canWrite && file.exists())
    {
        // give error message if !canWrite && file.exists()
        KMessageBox::error(this, i18n("You do not have permission to overwrite that scheme"), i18n("Error"));
    }
}

void ColorEditDialog::updateFromColorSchemes()
{
    // store colorscheme name in global settings
    KConfigGroup group(m_config, "General");
    group.writeEntry("ColorScheme", m_schemeName);

    for (int i = KColorScheme::View; i <= KColorScheme::Tooltip; ++i)
    {
        KConfigGroup group(m_config, colorSetGroupKey(i));
        group.writeEntry("BackgroundNormal", m_colorSchemes[i].background(KColorScheme::NormalBackground).color());
        group.writeEntry("BackgroundAlternate", m_colorSchemes[i].background(KColorScheme::AlternateBackground).color());
        group.writeEntry("ForegroundNormal", m_colorSchemes[i].foreground(KColorScheme::NormalText).color());
        group.writeEntry("ForegroundInactive", m_colorSchemes[i].foreground(KColorScheme::InactiveText).color());
        group.writeEntry("ForegroundActive", m_colorSchemes[i].foreground(KColorScheme::ActiveText).color());
        group.writeEntry("ForegroundLink", m_colorSchemes[i].foreground(KColorScheme::LinkText).color());
        group.writeEntry("ForegroundVisited", m_colorSchemes[i].foreground(KColorScheme::VisitedText).color());
        group.writeEntry("ForegroundNegative", m_colorSchemes[i].foreground(KColorScheme::NegativeText).color());
        group.writeEntry("ForegroundNeutral", m_colorSchemes[i].foreground(KColorScheme::NeutralText).color());
        group.writeEntry("ForegroundPositive", m_colorSchemes[i].foreground(KColorScheme::PositiveText).color());
        group.writeEntry("DecorationFocus", m_colorSchemes[i].decoration(KColorScheme::FocusColor).color());
        group.writeEntry("DecorationHover", m_colorSchemes[i].decoration(KColorScheme::HoverColor).color());
    }

    KConfigGroup WMGroup(m_config, "WM");
    WMGroup.writeEntry("activeBackground", m_wmColors.color(WindecoColors::ActiveBackground));
    WMGroup.writeEntry("activeForeground", m_wmColors.color(WindecoColors::ActiveForeground));
    WMGroup.writeEntry("inactiveBackground", m_wmColors.color(WindecoColors::InactiveBackground));
    WMGroup.writeEntry("inactiveForeground", m_wmColors.color(WindecoColors::InactiveForeground));
    WMGroup.writeEntry("activeBlend", m_wmColors.color(WindecoColors::ActiveBlend));
    WMGroup.writeEntry("inactiveBlend", m_wmColors.color(WindecoColors::InactiveBlend));
}

void ColorEditDialog::updateFromEffectsPage()
{
    if (m_disableUpdates)
    {
        // don't write the config as we are reading it!
        return;
    }

    KConfigGroup groupI(m_config, "ColorEffects:Inactive");
    KConfigGroup groupD(m_config, "ColorEffects:Disabled");

    // intensity
    groupI.writeEntry("IntensityEffect", inactiveIntensityBox->currentIndex());
    groupD.writeEntry("IntensityEffect", disabledIntensityBox->currentIndex());
    groupI.writeEntry("IntensityAmount", qreal(inactiveIntensitySlider->value() - 20) * 0.05);
    groupD.writeEntry("IntensityAmount", qreal(disabledIntensitySlider->value() - 20) * 0.05);

    // color
    groupI.writeEntry("ColorEffect", inactiveColorBox->currentIndex());
    groupD.writeEntry("ColorEffect", disabledColorBox->currentIndex());
    if (inactiveColorBox->currentIndex() > 1)
    {
        groupI.writeEntry("ColorAmount", qreal(inactiveColorSlider->value()) * 0.025);
    }
    else
    {
        groupI.writeEntry("ColorAmount", qreal(inactiveColorSlider->value() - 20) * 0.05);
    }
    if (disabledColorBox->currentIndex() > 1)
    {
        groupD.writeEntry("ColorAmount", qreal(disabledColorSlider->value()) * 0.025);
    }
    else
    {
        groupD.writeEntry("ColorAmount", qreal(disabledColorSlider->value() - 20) * 0.05);
    }
    groupI.writeEntry("Color", inactiveColorButton->color());
    groupD.writeEntry("Color", disabledColorButton->color());

    // contrast
    groupI.writeEntry("ContrastEffect", inactiveContrastBox->currentIndex());
    groupD.writeEntry("ContrastEffect", disabledContrastBox->currentIndex());
    groupI.writeEntry("ContrastAmount", qreal(inactiveContrastSlider->value()) * 0.05);
    groupD.writeEntry("ContrastAmount", qreal(disabledContrastSlider->value()) * 0.05);

    // enable/disable controls
    inactiveIntensitySlider->setDisabled(inactiveIntensityBox->currentIndex() == 0);
    disabledIntensitySlider->setDisabled(disabledIntensityBox->currentIndex() == 0);
    inactiveColorSlider->setDisabled(inactiveColorBox->currentIndex() == 0);
    disabledColorSlider->setDisabled(disabledColorBox->currentIndex() == 0);
    inactiveColorButton->setDisabled(inactiveColorBox->currentIndex() < 2);
    disabledColorButton->setDisabled(disabledColorBox->currentIndex() < 2);
    inactiveContrastSlider->setDisabled(inactiveContrastBox->currentIndex() == 0);
    disabledContrastSlider->setDisabled(disabledContrastBox->currentIndex() == 0);
}

void ColorEditDialog::setupColorTable()
{
    // first setup the common colors table
    commonColorTable->verticalHeader()->hide();
    commonColorTable->horizontalHeader()->hide();
    commonColorTable->setShowGrid(false);
    commonColorTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    int minWidth = QPushButton(i18n("Varies")).minimumSizeHint().width();
    commonColorTable->horizontalHeader()->setMinimumSectionSize(minWidth);
    commonColorTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

    for (int i = 0; i < 26; ++i)
    {
        KColorButton * button = new KColorButton(this);
        commonColorTable->setRowHeight(i, button->sizeHint().height());
        button->setObjectName(QString::number(i));
        connect(button, &KColorButton::changed, this, &ColorEditDialog::colorChanged);
        m_commonColorButtons << button;

        if (i > 8 && i < 18)
        {
            // Inactive Text row through Positive Text role all need a varies button
            QPushButton * variesButton = new QPushButton(NULL);
            variesButton->setText(i18n("Varies"));
            variesButton->setObjectName(QString::number(i));
            connect(variesButton, &QPushButton::clicked, this, &ColorEditDialog::variesClicked);

            QStackedWidget * widget = new QStackedWidget(this);
            widget->addWidget(button);
            widget->addWidget(variesButton);
            m_stackedWidgets.append(widget);

            commonColorTable->setCellWidget(i, 1, widget);
        }
        else
        {
            commonColorTable->setCellWidget(i, 1, button);
        }
    }

    // then the colorTable that the colorSets will use
    colorTable->verticalHeader()->hide();
    colorTable->horizontalHeader()->hide();
    colorTable->setShowGrid(false);
    colorTable->setRowCount(12);
    colorTable->horizontalHeader()->setMinimumSectionSize(minWidth);
    colorTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

    createColorEntry(i18n("Normal Background"),    QStringLiteral("BackgroundNormal"),    m_backgroundButtons, 0);
    createColorEntry(i18n("Alternate Background"), QStringLiteral("BackgroundAlternate"), m_backgroundButtons, 1);
    createColorEntry(i18n("Normal Text"),          QStringLiteral("ForegroundNormal"),    m_foregroundButtons, 2);
    createColorEntry(i18n("Inactive Text"),        QStringLiteral("ForegroundInactive"),  m_foregroundButtons, 3);
    createColorEntry(i18n("Active Text"),          QStringLiteral("ForegroundActive"),    m_foregroundButtons, 4);
    createColorEntry(i18n("Link Text"),            QStringLiteral("ForegroundLink"),      m_foregroundButtons, 5);
    createColorEntry(i18n("Visited Text"),         QStringLiteral("ForegroundVisited"),   m_foregroundButtons, 6);
    createColorEntry(i18n("Negative Text"),        QStringLiteral("ForegroundNegative"),  m_foregroundButtons, 7);
    createColorEntry(i18n("Neutral Text"),         QStringLiteral("ForegroundNeutral"),   m_foregroundButtons, 8);
    createColorEntry(i18n("Positive Text"),        QStringLiteral("ForegroundPositive"),  m_foregroundButtons, 9);
    createColorEntry(i18n("Focus Decoration"),     QStringLiteral("DecorationFocus"),     m_decorationButtons, 10);
    createColorEntry(i18n("Hover Decoration"),     QStringLiteral("DecorationHover"),     m_decorationButtons, 11);

    colorTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    colorTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

    updateColorSchemes();
    updateColorTable();
}

void ColorEditDialog::setCommonForeground(KColorScheme::ForegroundRole role, int stackIndex,
                                   int buttonIndex)
{
    QColor color = m_colorSchemes[KColorScheme::View].foreground(role).color();
    for (int i = KColorScheme::Window; i < KColorScheme::Tooltip; ++i)
    {
        if (i == KColorScheme::Selection && role == KColorScheme::InactiveText)
            break;

        if (m_colorSchemes[i].foreground(role).color() != color)
        {
            m_stackedWidgets[stackIndex]->setCurrentIndex(1);
            return;
        }
    }

    m_stackedWidgets[stackIndex]->setCurrentIndex(0);
    m_commonColorButtons[buttonIndex]->setColor(color);
    m_loadedSchemeHasUnsavedChanges = true;
}

void ColorEditDialog::setCommonDecoration(KColorScheme::DecorationRole role, int stackIndex,
                                   int buttonIndex)
{
    QColor color = m_colorSchemes[KColorScheme::View].decoration(role).color();
    for (int i = KColorScheme::Window; i < KColorScheme::Tooltip; ++i)
    {
        if (m_colorSchemes[i].decoration(role).color() != color)
        {
            m_stackedWidgets[stackIndex]->setCurrentIndex(1);
            return;
        }
    }

    m_stackedWidgets[stackIndex]->setCurrentIndex(0);
    m_commonColorButtons[buttonIndex]->setColor(color);
    m_loadedSchemeHasUnsavedChanges = true;
}

// inactive effects slots
void ColorEditDialog::on_inactiveIntensityBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_inactiveIntensitySlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_inactiveColorBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_inactiveColorSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_inactiveColorButton_changed(const QColor& color)
{
    Q_UNUSED( color );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_inactiveContrastBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_inactiveContrastSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

// disabled effects slots
void ColorEditDialog::on_disabledIntensityBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_disabledIntensitySlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_disabledColorBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_disabledColorSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_disabledColorButton_changed(const QColor& color)
{
    Q_UNUSED( color );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_disabledContrastBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::on_disabledContrastSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void ColorEditDialog::updateColorSchemes()
{
    m_colorSchemes.clear();

    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::View, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Window, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Button, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Selection, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Tooltip, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Complementary, m_config));

    m_wmColors.load(m_config);
}


void ColorEditDialog::createColorEntry(const QString &text, const QString &key, QList<KColorButton *> &list, int index)
{
    KColorButton *button = new KColorButton(this);
    button->setObjectName(QString::number(index));
    connect(button, &KColorButton::changed, this, &ColorEditDialog::colorChanged);
    list.append(button);

    m_colorKeys.insert(index, key);

    QTableWidgetItem *label = new QTableWidgetItem(text);
    colorTable->setItem(index, 0, label);
    colorTable->setCellWidget(index, 1, button);
    colorTable->setRowHeight(index, button->sizeHint().height());
}

void ColorEditDialog::variesClicked()
{
    // find which button was changed
    const int row = sender()->objectName().toInt();

    QColor color = QColorDialog::getColor(QColor(), this);
    if(color.isValid())
    {
        changeColor(row, color);
        m_stackedWidgets[row - 9]->setCurrentIndex(0);
    }
}
