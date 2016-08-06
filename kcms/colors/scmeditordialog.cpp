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

#include "scmeditordialog.h"
#include "scmeditoroptions.h"
#include "scmeditorcolors.h"
#include "scmeditoreffects.h"

#include <QDebug>
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QUrl>

#include <KConfigGroup>
#include <KColorScheme>
#include <KMessageBox>

#include <KNS3/UploadDialog>

SchemeEditorDialog::SchemeEditorDialog(const QString &path, QWidget *parent)
    : QDialog( parent )
    , m_disableUpdates(false)
    , m_filePath(path)
{

    m_config = KSharedConfig::openConfig(path);
    //const KSharedConfig oldconf = m_config.constData();
    setupUi(this);
    schemeKnsUploadButton->setIcon( QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")) );

    m_optionTab = new SchemeEditorOptions(m_config);
    m_colorTab = new SchemeEditorColors(m_config);
    m_disabledTab = new SchemeEditorEffects(m_config, QPalette::Disabled);
    m_inactiveTab = new SchemeEditorEffects(m_config, QPalette::Inactive);
    tabWidget->insertTab(OptionTab, m_optionTab, i18n("Options"));
    tabWidget->insertTab(ColorTab, m_colorTab, i18n("Colors"));
    tabWidget->insertTab(DisabledTab, m_disabledTab, i18n("Disabled"));

    connect(m_optionTab, &SchemeEditorOptions::changed, this, &SchemeEditorDialog::updateTabs);
    connect(m_colorTab, &SchemeEditorColors::changed, this, &SchemeEditorDialog::updateTabs);
    connect(m_disabledTab, &SchemeEditorEffects::changed, this, &SchemeEditorDialog::updateTabs);
    connect(m_inactiveTab, &SchemeEditorEffects::changed, this, &SchemeEditorDialog::updateTabs);

    buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    updateTabs();
}

void SchemeEditorDialog::on_schemeKnsUploadButton_clicked()
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
    QString path;
//FIXME    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
//        "color-schemes/" + schemeName + ".colors");
    if (path.isEmpty() ) // if the color scheme file wasn't found
    {
//        qDebug() << "path for color scheme " << m_schemeName << " couldn't be found";
        return;
    }

    // upload
    KNS3::UploadDialog dialog(QStringLiteral("colorschemes.knsrc"), this);
    dialog.setUploadFile(QUrl::fromLocalFile(path) );
    dialog.exec();
}

void SchemeEditorDialog::on_buttonBox_accepted()
{
    // prompt for the name to save as
    bool ok;
    QString schemeName = KConfigGroup(m_config, "General").readEntry("Name");
    QString name = QInputDialog::getText(this, i18n("Save Color Scheme"),
        i18n("&Enter a name for the color scheme:"), QLineEdit::Normal, schemeName, &ok);
    if (ok)
    {
        saveScheme(name);
    }
}


void SchemeEditorDialog::on_buttonBox_rejected()
{
    this->reject();
}

void SchemeEditorDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
    {
        m_config->markAsClean();;
        m_config->reparseConfiguration();
        updateTabs();
        buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
        buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
        schemeKnsUploadButton->setEnabled(false);
    }
}

void SchemeEditorDialog::saveScheme(const QString &name)
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
//        updateFromColorSchemes();
//        updateFromEffectsPage();
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

void SchemeEditorDialog::updateTabs(bool madeByUser)
{
    if (madeByUser)
    {
        qDebug() <<"update";
        buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
        buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
        schemeKnsUploadButton->setEnabled(true);
    }
    KConfigGroup group(m_config, "ColorEffects:Inactive");
    bool hideInactiveTab = group.readEntry("Enable", QVariant(true)).toBool();
    if ( hideInactiveTab )
    {
        tabWidget->insertTab(InactiveTab, m_inactiveTab, i18n("Inactive"));
    }
    else
    {
        tabWidget->removeTab(InactiveTab);
    }

    m_optionTab->updateValues();
    m_colorTab->updateValues();
    m_inactiveTab->updateValues();
    m_disabledTab->updateValues();
}
