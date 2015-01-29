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

#include "colorscm.h"

#include "../krdb/krdb.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QTimer>
#include <QHeaderView>
#include <QInputDialog>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QPainter>
#include <QBitmap>
#include <QtDBus/QtDBus>
#include <QColorDialog>

#include <KAboutData>
#include <KColorButton>
#include <KConfigGroup>
#include <KColorUtils>
#include <KColorScheme>
#include <KMessageBox>
#include <KPluginFactory>
#include <kio/netaccess.h>
#include <kio/deletejob.h>
#include <kio/jobuidelegate.h>
#include <KNewStuff3/KNS3/DownloadDialog>
#include <KNewStuff3/KNS3/UploadDialog>

K_PLUGIN_FACTORY( KolorFactory, registerPlugin<KColorCm>(); )
K_EXPORT_PLUGIN( KolorFactory("kcmcolors") )

//BEGIN WindecoColors
KColorCm::WindecoColors::WindecoColors(const KSharedConfigPtr &config)
{
    load(config);
}

void KColorCm::WindecoColors::load(const KSharedConfigPtr &config)
{
    // NOTE: keep this in sync with kdelibs/kdeui/kernel/kglobalsettings.cpp
    KConfigGroup group(config, "WM");
    m_colors[ActiveBackground] = group.readEntry("activeBackground", QColor(48, 174, 232));
    m_colors[ActiveForeground] = group.readEntry("activeForeground", QColor(255, 255, 255));
    m_colors[InactiveBackground] = group.readEntry("inactiveBackground", QColor(224, 223, 222));
    m_colors[InactiveForeground] = group.readEntry("inactiveForeground", QColor(75, 71, 67));
    m_colors[ActiveBlend] = group.readEntry("activeBlend", m_colors[ActiveForeground]);
    m_colors[InactiveBlend] = group.readEntry("inactiveBlend", m_colors[InactiveForeground]);
}

QColor KColorCm::WindecoColors::color(WindecoColors::Role role) const
{
    return m_colors[role];
}
//END WindecoColors

KColorCm::KColorCm(QWidget *parent, const QVariantList &)
    : KCModule( parent ), m_disableUpdates(false),
      m_loadedSchemeHasUnsavedChanges(false), m_dontLoadSelectedScheme(false),
      m_previousSchemeItem(0)
{
    KAboutData* about = new KAboutData(
        QStringLiteral("kcmcolors"), i18n("Colors"), QStringLiteral("1.0"), QString(),
        KAboutLicense::GPL,
        i18n("(c) 2007 Matthew Woehlke")
    );
    about->addAuthor( i18n("Matthew Woehlke"), QString(),
                     QStringLiteral("mw_triad@users.sourceforge.net") );
    about->addAuthor( i18n("Jeremy Whiting"), QString(), QStringLiteral("jpwhiting@kde.org"));
    setAboutData( about );

    m_config = KSharedConfig::openConfig("kdeglobals");

    setupUi(this);
    schemeKnsButton->setIcon( QIcon::fromTheme("get-hot-new-stuff") );
    schemeKnsUploadButton->setIcon( QIcon::fromTheme("get-hot-new-stuff") );
    connect(colorSet, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KColorCm::updateColorTable);
    connect(schemeList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(loadScheme(QListWidgetItem*,QListWidgetItem*)));
    connect(applyToAlien, &QCheckBox::toggled, this, &KColorCm::emitChanged);

    // only needs to be called once
    setupColorTable();
}

KColorCm::~KColorCm()
{
    m_config->markAsClean();
}

void KColorCm::populateSchemeList()
{
    // clear the list in case this is being called from reset button click
    schemeList->clear();

    // add entries
    QIcon icon;

    QStringList schemeFiles;
    const QStringList schemeDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "color-schemes", QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString &dir, schemeDirs)
    {
        const QStringList fileNames = QDir(dir).entryList(QStringList()<<"*.colors");
        Q_FOREACH (const QString &file, fileNames)
        {
            if( !schemeFiles.contains("color-schemes/"+file))
            {
                schemeFiles.append("color-schemes/"+file);
            }
        }
    }
    for (QStringList::Iterator it = schemeFiles.begin(); it != schemeFiles.end(); ++it )
    {
        *it = QStandardPaths::locate(QStandardPaths::GenericDataLocation, *it);
    }

    for (int i = 0; i < schemeFiles.size(); ++i)
    {
        // get the file name
        const QString filename = schemeFiles.at(i);
        const QFileInfo info(filename);

        // add the entry
        KSharedConfigPtr config = KSharedConfig::openConfig(filename);
        icon = createSchemePreviewIcon(config);
        KConfigGroup group(config, "General");
        const QString name = group.readEntry("Name", info.baseName());
        QListWidgetItem * newItem = new QListWidgetItem(icon, name);
        // stash the file basename for use later
        newItem->setData(Qt::UserRole, info.baseName());
        schemeList->addItem(newItem);
    }
    schemeList->sortItems();

    // add default entry (do this here so that the current and default entry appear at the top)
    m_config->setReadDefaults(true);
    icon = createSchemePreviewIcon(m_config);
    schemeList->insertItem(0, new QListWidgetItem(icon, i18nc("Default color scheme", "Default")));
    m_config->setReadDefaults(false);

    // add current scheme entry
    icon = createSchemePreviewIcon(m_config);
    QListWidgetItem *currentitem = new QListWidgetItem(icon, i18nc("Current color scheme", "Current"));
    schemeList->insertItem(0, currentitem);
    schemeList->blockSignals(true); // don't emit changed signals
    schemeList->setCurrentItem(currentitem);
    schemeList->blockSignals(false);
}

void KColorCm::updatePreviews()
{
    schemePreview->setPalette(m_config);
    colorPreview->setPalette(m_config);
    setPreview->setPalette(m_config, (KColorScheme::ColorSet)(colorSet->currentIndex() - 1));
    inactivePreview->setPalette(m_config, QPalette::Inactive);
    disabledPreview->setPalette(m_config, QPalette::Disabled);
}

void KColorCm::updateEffectsPage()
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

void KColorCm::loadScheme(KSharedConfigPtr config) // const QString &path)
{
    KSharedConfigPtr temp = m_config;
    m_config = config;

    updateColorSchemes();
    updateEffectsPage(); // intentionally before swapping back m_config
    updatePreviews();

    m_config = temp;
    updateFromColorSchemes();
    updateFromEffectsPage();
    updateFromOptions();
    updateColorTable();

    m_loadedSchemeHasUnsavedChanges = false;
    //m_changed = false;
}

void KColorCm::selectPreviousSchemeAgain()
{
    m_dontLoadSelectedScheme = true;
    schemeList->setCurrentItem(m_previousSchemeItem);
    m_dontLoadSelectedScheme = false;
}

void KColorCm::loadScheme(QListWidgetItem *currentItem, QListWidgetItem *previousItem)
{
    m_previousSchemeItem = previousItem;

    if (m_dontLoadSelectedScheme)
    {
        return;
    }

    if (currentItem != NULL)
    {
        if (m_loadedSchemeHasUnsavedChanges) // if changes made to loaded scheme
        {
            if (KMessageBox::Continue != KMessageBox::warningContinueCancel(this,
                i18n("Selecting another scheme will discard any changes you have made"),
                i18n("Are you sure?"),
                KStandardGuiItem::cont(),
                KStandardGuiItem::cancel(),
                "noDiscardWarning")) // if the user decides to cancel
            {
                QTimer::singleShot(0, this, SLOT(selectPreviousSchemeAgain()));
                return;
            }
        }

        // load it

        const QString name = currentItem->text();
        m_currentColorScheme = name;
        const QString fileBaseName = currentItem->data(Qt::UserRole).toString();
        if (name == i18nc("Default color scheme", "Default"))
        {
            schemeRemoveButton->setEnabled(false);
            schemeKnsUploadButton->setEnabled(false);

            KSharedConfigPtr config = m_config;
            config->setReadDefaults(true);
            loadScheme(config);
            config->setReadDefaults(false);
            // load the default scheme
            emit changed(true);
        }
        else if (name == i18nc("Current color scheme", "Current"))
        {
            schemeRemoveButton->setEnabled(false);
            schemeKnsUploadButton->setEnabled(false);
            loadInternal(false);
        }
        else
        {
            const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                "color-schemes/" + fileBaseName + ".colors");

            const int permissions = QFile(path).permissions();
            const bool canWrite = (permissions & QFile::WriteUser);
            qDebug() << "checking permissions of " << path;
            schemeRemoveButton->setEnabled(canWrite);
            schemeKnsUploadButton->setEnabled(true);

            KSharedConfigPtr config = KSharedConfig::openConfig(path);
            loadScheme(config);

            emit changed(true);
        }
    }
}

void KColorCm::on_schemeRemoveButton_clicked()
{
    if (schemeList->currentItem() != NULL)
    {
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            "color-schemes/" + schemeList->currentItem()->data(Qt::UserRole).toString() +
            ".colors");
        KIO::DeleteJob *job = KIO::del(QUrl::fromLocalFile(path));
        job->uiDelegate()->setParent(this);
        if (job->exec())
        {
            delete schemeList->takeItem(schemeList->currentRow());
        }
        else
        {
            // deletion failed, so show an error message
            KMessageBox::error(this, i18n("You do not have permission to delete that scheme"), i18n("Error"));
        }
    }
}

void KColorCm::on_schemeImportButton_clicked()
{
    // get the path to the scheme to import
    QUrl url = QUrl::fromLocalFile(QFileDialog::getOpenFileName(this, i18n("Import Color Scheme")));

    if(url.isValid())
    {
        // TODO: possibly untar or uncompress it
        // open it

        // load the scheme
        KSharedConfigPtr config = KSharedConfig::openConfig(url.path());

        if (config->groupList().contains("Color Scheme"))
        {
            if (KMessageBox::Continue != KMessageBox::warningContinueCancel(this,
                i18n("The scheme you have selected appears to be a KDE3 scheme.\n\n"
                     "KDE will attempt to import this scheme, however many color roles have been added since KDE3. "
                     "Some manual work will likely be required.\n\n"
                     "This scheme will not be saved automatically."),
                i18n("Notice")))
            {
                return;
            }

            // convert KDE3 scheme to KDE4 scheme
            KConfigGroup g(config, "Color Scheme");
            KConfigGroup generalGroup(KSharedConfig::openConfig(), "General");

            colorSet->setCurrentIndex(0);
            contrastSlider->setValue(g.readEntry("contrast", KColorScheme::contrast()));
            shadeSortedColumn->setChecked(g.readEntry("shadeSortColumn", generalGroup.readEntry("shadeSortColumn", true)));

            m_commonColorButtons[0]->setColor(g.readEntry("windowBackground", m_colorSchemes[KColorScheme::View].background().color()));
            m_commonColorButtons[1]->setColor(g.readEntry("windowForeground", m_colorSchemes[KColorScheme::View].foreground().color()));
            m_commonColorButtons[2]->setColor(g.readEntry("background", m_colorSchemes[KColorScheme::Window].background().color()));
            m_commonColorButtons[3]->setColor(g.readEntry("foreground", m_colorSchemes[KColorScheme::Window].foreground().color()));
            m_commonColorButtons[4]->setColor(g.readEntry("buttonBackground", m_colorSchemes[KColorScheme::Button].background().color()));
            m_commonColorButtons[5]->setColor(g.readEntry("buttonForeground", m_colorSchemes[KColorScheme::Button].foreground().color()));
            m_commonColorButtons[6]->setColor(g.readEntry("selectBackground", m_colorSchemes[KColorScheme::Selection].background().color()));
            m_commonColorButtons[7]->setColor(g.readEntry("selectForeground", m_colorSchemes[KColorScheme::Selection].foreground().color()));
            m_commonColorButtons[8]->setColor(KColorUtils::mix(m_colorSchemes[KColorScheme::Selection].foreground().color(),
                                                               m_colorSchemes[KColorScheme::Selection].background().color()));
            m_commonColorButtons[9]->setColor(KColorUtils::mix(m_colorSchemes[KColorScheme::View].foreground().color(),
                                                               m_colorSchemes[KColorScheme::View].background().color()));
            // doesn't exist in KDE3: 10 ActiveText
            m_commonColorButtons[11]->setColor(g.readEntry("linkColor", m_colorSchemes[KColorScheme::View].foreground(KColorScheme::LinkText).color()));
            m_commonColorButtons[12]->setColor(g.readEntry("visitedLinkColor", m_colorSchemes[KColorScheme::View].foreground(KColorScheme::VisitedText).color()));
            // doesn't exist in KDE3: 13-15 PositiveText, NeutralText, NegativeText
            m_commonColorButtons[16]->setColor(g.readEntry("windowForeground", m_colorSchemes[KColorScheme::View].decoration(KColorScheme::FocusColor).color()));
            m_commonColorButtons[17]->setColor(g.readEntry("selectBackground", m_colorSchemes[KColorScheme::View].decoration(KColorScheme::HoverColor).color()));
            m_commonColorButtons[18]->setColor(g.readEntry("windowBackground", m_colorSchemes[KColorScheme::Tooltip].background().color()));
            m_commonColorButtons[19]->setColor(g.readEntry("windowForeground", m_colorSchemes[KColorScheme::Tooltip].foreground().color()));
            m_commonColorButtons[20]->setColor(g.readEntry("activeBackground", m_wmColors.color(WindecoColors::ActiveBackground)));
            m_commonColorButtons[21]->setColor(g.readEntry("activeForeground", m_wmColors.color(WindecoColors::ActiveForeground)));
            m_commonColorButtons[22]->setColor(g.readEntry("activeBlend", m_wmColors.color(WindecoColors::ActiveBlend)));
            m_commonColorButtons[23]->setColor(g.readEntry("inactiveBackground", m_wmColors.color(WindecoColors::InactiveBackground)));
            m_commonColorButtons[24]->setColor(g.readEntry("inactiveForeground", m_wmColors.color(WindecoColors::InactiveForeground)));
            m_commonColorButtons[25]->setColor(g.readEntry("inactiveBlend", m_wmColors.color(WindecoColors::InactiveBlend)));

            colorSet->setCurrentIndex(1);
            m_backgroundButtons[KColorScheme::AlternateBackground]->setColor(g.readEntry("alternateBackground",
                                                                                         m_colorSchemes[KColorScheme::View].background(KColorScheme::AlternateBackground).color()));
            colorSet->setCurrentIndex(0);
        }
        else
        {
            // load KDE4 scheme
            loadScheme(config);

            // save it
            saveScheme(url.fileName());
        }
    }
}

void KColorCm::on_schemeKnsButton_clicked()
{
    KNS3::DownloadDialog dialog("colorschemes.knsrc", this);
    dialog.exec();
    if ( ! dialog.changedEntries().isEmpty() )
    {
        populateSchemeList();
    }
}

void KColorCm::on_schemeKnsUploadButton_clicked()
{
    if (schemeList->currentItem() != NULL)
    {
        // check if the currently loaded scheme has unsaved changes
        if (m_loadedSchemeHasUnsavedChanges)
        {
            KMessageBox::sorry(this, i18n("Please save the color scheme before uploading it."),
                               i18n("Please save"));
            return;
        }

        // find path
        const QString basename = schemeList->currentItem()->data(Qt::UserRole).toString();
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            "color-schemes/" + basename + ".colors");
        if (path.isEmpty() ) // if the color scheme file wasn't found
        {
            qDebug() << "path for color scheme " << basename << " couldn't be found";
            return;
        }

        // upload
        KNS3::UploadDialog dialog("colorschemes.knsrc", this);
        dialog.setUploadFile(QUrl::fromLocalFile(path) );
        dialog.exec();
    }
}

void KColorCm::on_schemeSaveButton_clicked()
{
    QString previousName;
    if (schemeList->currentItem() != NULL && schemeList->currentRow() > 1)
    {
        previousName = schemeList->currentItem()->data(Qt::DisplayRole).toString();
    }
    // prompt for the name to save as
    bool ok;
    QString name = QInputDialog::getText(this, i18n("Save Color Scheme"),
        i18n("&Enter a name for the color scheme:"), QLineEdit::Normal, previousName, &ok);
    if (ok)
    {
        saveScheme(name);
    }
}

void KColorCm::saveScheme(const QString &name)
{
    QString filename = name;
    filename.remove('\''); // So Foo's does not become FooS
    QRegExp fixer("[\\W,.-]+(.?)");
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
            if(ret == KMessageBox::No){
                QList<QListWidgetItem*> foundItems = schemeList->findItems(name, Qt::MatchExactly);
                if (foundItems.size() == 1)
                    schemeList->setCurrentRow(schemeList->row(foundItems[0]));
                return;
            }
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

        QList<QListWidgetItem*> foundItems = schemeList->findItems(name, Qt::MatchExactly);
        QIcon icon = createSchemePreviewIcon(m_config);
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
        }

        // set m_config back to the system one
        m_config = temp;

        // store colorscheme name in global settings
        m_currentColorScheme = name;
        group = KConfigGroup(m_config, "General");
        group.writeEntry("ColorScheme", m_currentColorScheme);

        emit changed(true);
    }
    else if (!canWrite && file.exists())
    {
        // give error message if !canWrite && file.exists()
        KMessageBox::error(this, i18n("You do not have permission to overwrite that scheme"), i18n("Error"));
    }
}

void KColorCm::createColorEntry(const QString &text, const QString &key, QList<KColorButton *> &list, int index)
{
    KColorButton *button = new KColorButton(this);
    button->setObjectName(QString::number(index));
    connect(button, &KColorButton::changed, this, &KColorCm::colorChanged);
    list.append(button);

    m_colorKeys.insert(index, key);

    QTableWidgetItem *label = new QTableWidgetItem(text);
    colorTable->setItem(index, 0, label);
    colorTable->setCellWidget(index, 1, button);
    colorTable->setRowHeight(index, button->sizeHint().height());
}

void KColorCm::variesClicked()
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

QPixmap KColorCm::createSchemePreviewIcon(const KSharedConfigPtr &config)
{
    const WindecoColors wm(config);
    const uchar bits1[] = { 0xff, 0xff, 0xff, 0x2c, 0x16, 0x0b };
    const uchar bits2[] = { 0x68, 0x34, 0x1a, 0xff, 0xff, 0xff };
    const QSize bitsSize(24,2);
    const QBitmap b1 = QBitmap::fromData(bitsSize, bits1);
    const QBitmap b2 = QBitmap::fromData(bitsSize, bits2);

    QPixmap pixmap(23, 16);
    pixmap.fill(Qt::black); // ### use some color other than black for borders?

    QPainter p(&pixmap);

    KColorScheme windowScheme(QPalette::Active, KColorScheme::Window, config);
    p.fillRect( 1,  1, 7, 7, windowScheme.background());
    p.fillRect( 2,  2, 5, 2, QBrush(windowScheme.foreground().color(), b1));

    KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button, config);
    p.fillRect( 8,  1, 7, 7, buttonScheme.background());
    p.fillRect( 9,  2, 5, 2, QBrush(buttonScheme.foreground().color(), b1));

    p.fillRect(15,  1, 7, 7, wm.color(WindecoColors::ActiveBackground));
    p.fillRect(16,  2, 5, 2, QBrush(wm.color(WindecoColors::ActiveForeground), b1));

    KColorScheme viewScheme(QPalette::Active, KColorScheme::View, config);
    p.fillRect( 1,  8, 7, 7, viewScheme.background());
    p.fillRect( 2, 12, 5, 2, QBrush(viewScheme.foreground().color(), b2));

    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection, config);
    p.fillRect( 8,  8, 7, 7, selectionScheme.background());
    p.fillRect( 9, 12, 5, 2, QBrush(selectionScheme.foreground().color(), b2));

    p.fillRect(15,  8, 7, 7, wm.color(WindecoColors::InactiveBackground));
    p.fillRect(16, 12, 5, 2, QBrush(wm.color(WindecoColors::InactiveForeground), b2));

    p.end();

    return pixmap;
}

void KColorCm::updateColorSchemes()
{
    m_colorSchemes.clear();

    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::View, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Window, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Button, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Selection, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Tooltip, m_config));

    m_wmColors.load(m_config);
}

void KColorCm::updateFromColorSchemes()
{
    // store colorscheme name in global settings
    KConfigGroup group(m_config, "General");
    group.writeEntry("ColorScheme", m_currentColorScheme);

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

void KColorCm::updateFromOptions()
{
    KConfigGroup groupK(m_config, "KDE");
    groupK.writeEntry("contrast", contrastSlider->value());

    KConfigGroup groupG(m_config, "General");
    groupG.writeEntry("shadeSortColumn", shadeSortedColumn->isChecked());

    KConfigGroup groupI(m_config, "ColorEffects:Inactive");
    groupI.writeEntry("Enable", useInactiveEffects->isChecked());
    // only write this setting if it is not the default; this way we can change the default more easily in later KDE
    // the setting will still written by explicitly checking/unchecking the box
    if (inactiveSelectionEffect->isChecked())
    {
        groupI.writeEntry("ChangeSelectionColor", true);
    }
    else
    {
        groupI.deleteEntry("ChangeSelectionColor");
    }
}

void KColorCm::updateFromEffectsPage()
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

void KColorCm::setupColorTable()
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
        connect(button, &KColorButton::changed, this, &KColorCm::colorChanged);
        m_commonColorButtons << button;

        if (i > 8 && i < 18)
        {
            // Inactive Text row through Positive Text role all need a varies button
            QPushButton * variesButton = new QPushButton(NULL);
            variesButton->setText(i18n("Varies"));
            variesButton->setObjectName(QString::number(i));
            connect(variesButton, &QPushButton::clicked, this, &KColorCm::variesClicked);

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

    createColorEntry(i18n("Normal Background"),    "BackgroundNormal",    m_backgroundButtons, 0);
    createColorEntry(i18n("Alternate Background"), "BackgroundAlternate", m_backgroundButtons, 1);
    createColorEntry(i18n("Normal Text"),          "ForegroundNormal",    m_foregroundButtons, 2);
    createColorEntry(i18n("Inactive Text"),        "ForegroundInactive",  m_foregroundButtons, 3);
    createColorEntry(i18n("Active Text"),          "ForegroundActive",    m_foregroundButtons, 4);
    createColorEntry(i18n("Link Text"),            "ForegroundLink",      m_foregroundButtons, 5);
    createColorEntry(i18n("Visited Text"),         "ForegroundVisited",   m_foregroundButtons, 6);
    createColorEntry(i18n("Negative Text"),        "ForegroundNegative",  m_foregroundButtons, 7);
    createColorEntry(i18n("Neutral Text"),         "ForegroundNeutral",   m_foregroundButtons, 8);
    createColorEntry(i18n("Positive Text"),        "ForegroundPositive",  m_foregroundButtons, 9);
    createColorEntry(i18n("Focus Decoration"),     "DecorationFocus",     m_decorationButtons, 10);
    createColorEntry(i18n("Hover Decoration"),     "DecorationHover",     m_decorationButtons, 11);

    colorTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    colorTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

    updateColorSchemes();
    updateColorTable();
}

void KColorCm::setCommonForeground(KColorScheme::ForegroundRole role, int stackIndex,
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

void KColorCm::setCommonDecoration(KColorScheme::DecorationRole role, int stackIndex,
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

void KColorCm::updateColorTable()
{
    // subtract one here since the 0 item  is "Common Colors"
    const int currentSet = colorSet->currentIndex() - 1;

    if (currentSet == -1)
    {
        // common colors is selected
        stackColors->setCurrentIndex(0);
        stackPreview->setCurrentIndex(0);

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
        setPreview->setPalette(m_config, (KColorScheme::ColorSet)currentSet);
        stackColors->setCurrentIndex(1);
        stackPreview->setCurrentIndex(1);

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

void KColorCm::colorChanged( const QColor &newColor )
{
    // find which button was changed
    const int row = sender()->objectName().toInt();
    changeColor(row, newColor);
}

void KColorCm::changeColor(int row, const QColor &newColor)
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

    QIcon icon = createSchemePreviewIcon(m_config);
    schemeList->item(0)->setIcon(icon);
    updateColorSchemes();
    updatePreviews();

    m_loadedSchemeHasUnsavedChanges = true;
    m_currentColorScheme = i18nc("Current color scheme", "Current");
    KConfigGroup group(m_config, "General");
    group.writeEntry("ColorScheme", m_currentColorScheme);
    schemeRemoveButton->setEnabled(false);
    schemeKnsUploadButton->setEnabled(false);
    schemeList->blockSignals(true); // don't emit changed signals
    schemeList->setCurrentRow(0);
    schemeList->blockSignals(false);

    emit changed(true);
}

QString KColorCm::colorSetGroupKey(int colorSet)
{
    QString group;
    switch (colorSet) {
        case KColorScheme::Window:
            group = "Colors:Window";
            break;
        case KColorScheme::Button:
            group = "Colors:Button";
            break;
        case KColorScheme::Selection:
            group = "Colors:Selection";
            break;
        case KColorScheme::Tooltip:
            group = "Colors:Tooltip";
            break;
        default:
            group = "Colors:View";
    }
    return group;
}

void KColorCm::on_contrastSlider_valueChanged(int value)
{
    KConfigGroup group(m_config, "KDE");
    group.writeEntry("contrast", value);

    updatePreviews();

    emit changed(true);
}

void KColorCm::on_shadeSortedColumn_stateChanged(int state)
{
    KConfigGroup group(m_config, "General");
    group.writeEntry("shadeSortColumn", bool(state != Qt::Unchecked));

    emit changed(true);
}

void KColorCm::on_useInactiveEffects_stateChanged(int state)
{
    KConfigGroup group(m_config, "ColorEffects:Inactive");
    group.writeEntry("Enable", bool(state != Qt::Unchecked));

    m_disableUpdates = true;
    printf("re-init\n");
    inactiveSelectionEffect->setChecked(group.readEntry("ChangeSelectionColor", bool(state != Qt::Unchecked)));
    m_disableUpdates = false;

    emit changed(true);
}

void KColorCm::on_inactiveSelectionEffect_stateChanged(int state)
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

void KColorCm::load()
{
    loadInternal(true);

    // get colorscheme name from global settings
    KConfigGroup group(m_config, "General");
    m_currentColorScheme = group.readEntry("ColorScheme");
    if (m_currentColorScheme == i18nc("Current color scheme", "Current"))
    {
        m_loadedSchemeHasUnsavedChanges = true;
    }
    QList<QListWidgetItem*> itemList = schemeList->findItems(m_currentColorScheme, Qt::MatchExactly);
    if(!itemList.isEmpty()) // "Current" is already selected, so don't handle the case that itemList is empty
        schemeList->setCurrentItem(itemList.at(0));

    KConfig      cfg("kcmdisplayrc", KConfig::NoGlobals);
    group = KConfigGroup(&cfg, "X11");

    applyToAlien->blockSignals(true); // don't emit SIGNAL(toggled(bool)) which would call SLOT(emitChanged())
    applyToAlien->setChecked(group.readEntry("exportKDEColors", true));
    applyToAlien->blockSignals(false);
}

void KColorCm::loadOptions()
{
    KConfigGroup generalGroup(KSharedConfig::openConfig(), "General");
    contrastSlider->setValue(KColorScheme::contrast());
    shadeSortedColumn->setChecked(generalGroup.readEntry("shadeSortColumn", true));

    KConfigGroup group(m_config, "ColorEffects:Inactive");
    useInactiveEffects->setChecked(group.readEntry("Enable", false));
    // NOTE: keep this in sync with kdelibs/kdeui/colors/kcolorscheme.cpp
    // NOTE: remove extra logic from updateFromOptions and on_useInactiveEffects_stateChanged when this changes!
    inactiveSelectionEffect->setChecked(group.readEntry("ChangeSelectionColor", group.readEntry("Enable", true)));
}

void KColorCm::loadInternal(bool loadOptions_)
{
    // clean the config, in case we have changed the in-memory kconfig
    m_config->markAsClean();
    m_config->reparseConfiguration();

    // update the color table
    updateColorSchemes();
    updateColorTable();

    // fill in the color scheme list
    populateSchemeList();

    if (loadOptions_)
        loadOptions();

    updateEffectsPage();

    updatePreviews();

    m_loadedSchemeHasUnsavedChanges = false;

    emit changed(false);
}

void KColorCm::save()
{
    QIcon icon = createSchemePreviewIcon(m_config);
    schemeList->item(0)->setIcon(icon);

    KConfigGroup groupI(m_config, "ColorEffects:Inactive");

    groupI.writeEntry("Enable", useInactiveEffects->isChecked());
    groupI.writeEntry("IntensityEffect", inactiveIntensityBox->currentIndex());
    groupI.writeEntry("ColorEffect", inactiveColorBox->currentIndex());
    groupI.writeEntry("ContrastEffect", inactiveContrastBox->currentIndex());

    m_config->sync();
    KConfig      cfg("kcmdisplayrc", KConfig::NoGlobals);
    KConfigGroup displayGroup(&cfg, "X11");

    displayGroup.writeEntry("exportKDEColors", applyToAlien->isChecked());
    cfg.sync();

    runRdb(KRdbExportQtColors | KRdbExportGtkTheme | ( applyToAlien->isChecked() ? KRdbExportColors : 0 ) );

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange" );
    QList<QVariant> args;
    args.append(0);//previous KGlobalSettings::PaletteChanged. This is now private API in khintsettings
    args.append(0);//unused in palette changed but needed for the DBus signature
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
    if (qApp->platformName() == QStringLiteral("xcb")) {
        // Send signal to all kwin instances
        QDBusMessage message =
        QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }

    emit changed(false);
}

void KColorCm::defaults()
{
    // Switch to default scheme
    for(int i = 0; i < schemeList->count(); ++i) {
        QListWidgetItem *item = schemeList->item(i);
        if(item->text() == i18nc("Default color scheme", "Default")) {
            // If editing the default scheme, force a reload, else select the default scheme
            if(schemeList->currentItem() == item)
                loadScheme(item, item);
            else
                schemeList->setCurrentItem(item);
            m_currentColorScheme = item->text();
            break;
        }
    }

    // Reset options (not part of scheme)
    m_config->setReadDefaults(true);
    loadOptions();
    m_config->setReadDefaults(false);
    applyToAlien->setChecked(Qt::Checked);

    KCModule::defaults();
    emit changed(true);
}

void KColorCm::emitChanged()
{
    emit changed(true);
}

// inactive effects slots
void KColorCm::on_inactiveIntensityBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_inactiveIntensitySlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_inactiveColorBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_inactiveColorSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_inactiveColorButton_changed(const QColor& color)
{
    Q_UNUSED( color );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_inactiveContrastBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_inactiveContrastSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    inactivePreview->setPalette(m_config, QPalette::Inactive);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

// disabled effects slots
void KColorCm::on_disabledIntensityBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_disabledIntensitySlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_disabledColorBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_disabledColorSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_disabledColorButton_changed(const QColor& color)
{
    Q_UNUSED( color );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_disabledContrastBox_currentIndexChanged(int index)
{
    Q_UNUSED( index );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

void KColorCm::on_disabledContrastSlider_valueChanged(int value)
{
    Q_UNUSED( value );

    updateFromEffectsPage();
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    m_loadedSchemeHasUnsavedChanges = true;

    emit changed(true);
}

#include "colorscm.moc"
