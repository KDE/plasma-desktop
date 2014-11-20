/*
  Copyright (c) 2008 Andrew Lake <jamboarder@yahoo.com>
  Copyright (c) 2010 Jeremy Whiting <jpwhiting@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "desktopthemedetails.h"

#include "thememodel.h"

#include <QComboBox>
#include <QDir>
#include <QTextStream>
#include <QFileDialog>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KFileDialog>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KZip>
#include <KUrl>
#include <kio/netaccess.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/job.h>
#include <qstandardpaths.h>
#include <KLocalizedString>

#include <QDebug>

struct ThemeItemNameType {
        const char* m_type;
        const char* m_displayItemName;
        const char* m_themeItemPath;
        const char* m_iconName;
};

const ThemeItemNameType themeCollectionName[] = {
    { "Color Scheme", I18N_NOOP2("plasma name", "Color Scheme"),"colors", "preferences-desktop-color"},
    { "Panel Background", I18N_NOOP2("plasma name", "Panel Background"),"widgets/panel-background", "plasma"},
    { "Kickoff", I18N_NOOP2("plasma name", "Kickoff"), "dialogs/kickoff", "kde"},
    { "Task Items", I18N_NOOP2("plasma name", "Task Items"), "widgets/tasks", "preferences-system-windows"},
    { "Widget Background", I18N_NOOP2("plasma name", "Widget Background"), "widgets/background", "plasma"},
    { "Translucent Background", I18N_NOOP2("plasma name", "Translucent Background"), "widgets/translucentbackground", "plasma"},
    { "Dialog Background", I18N_NOOP2("plasma name", "Dialog Background"), "dialogs/background", "plasma"},
    { "Analog Clock", I18N_NOOP2("plasma name", "Analog Clock"), "widgets/clock", "chronometer"},
    { "Notes", I18N_NOOP2("plasma name", "Notes"), "widgets/notes", "view-pim-notes"},
    { "Tooltip", I18N_NOOP2("plasma name", "Tooltip"), "widgets/tooltip", "plasma"},
    { "Pager", I18N_NOOP2("plasma name", "Pager"), "widgets/pager", "plasma"},
    { "Run Command Dialog", I18N_NOOP2("plasma name", "Run Command Dialog"), "dialogs/krunner", "system-run"},
    { "Shutdown Dialog", I18N_NOOP2("plasma name", "Shutdown Dialog"), "dialogs/shutdowndialog", "system-shutdown"},
    { "Icons", I18N_NOOP2("plasma name", "Icons"), "icons/", "preferences-desktop-icons"},
    { 0, 0,0,0 } // end of data
};


DesktopThemeDetails::DesktopThemeDetails(QWidget* parent)
    : QWidget(parent),
      m_themeModel(0)

{
    setWindowIcon(QIcon::fromTheme("preferences-desktop"));
    setupUi(this);

    QFont font = QFont();
    //font.setBold(true);
    font.setPointSize(1.2*font.pointSize());
    m_themeInfoName->setFont(font);

    m_enableAdvanced->setChecked(false);
    toggleAdvancedVisible();


    m_themeModel = new ThemeModel(this);
    m_theme->setModel(m_themeModel);
    m_theme->setItemDelegate(new ThemeDelegate(m_theme));

    reloadConfig();

    connect(m_theme->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(themeSelectionChanged(QItemSelection,QItemSelection)));

    connect(m_enableAdvanced, &QCheckBox::toggled, this, &DesktopThemeDetails::toggleAdvancedVisible);
    connect(m_removeThemeButton, &QPushButton::clicked, this, &DesktopThemeDetails::removeTheme);
    connect(m_exportThemeButton, &QPushButton::clicked, this, &DesktopThemeDetails::exportTheme);

    connect(m_newThemeName, &KLineEdit::editingFinished, this, &DesktopThemeDetails::newThemeInfoChanged);

    m_baseTheme = "default";
    m_themeCustomized = false;
    resetThemeDetails();
    adjustSize();
}

DesktopThemeDetails::~DesktopThemeDetails()
{
    cleanup();
}

void DesktopThemeDetails::cleanup()
{
}

void DesktopThemeDetails::reloadConfig()
{
    // Theme
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig("plasmarc"), "Theme");
    const QString theme = cfg.readEntry("name", "default");
    m_theme->setCurrentIndex(m_themeModel->indexOf(theme));
}

void DesktopThemeDetails::resetToDefaultTheme()
{
    m_theme->setCurrentIndex(m_themeModel->indexOf("default"));
}

void DesktopThemeDetails::reloadModel()
{
    m_themeModel->reload();
}

void DesktopThemeDetails::save()
{
    QString themeRoot;
    KStandardDirs dirs;
    if (m_newThemeName->text().isEmpty()) {
        themeRoot = ".customized";
        //Toggle customized theme directory name to ensure theme reload
        if (QDir(dirs.locateLocal("data", "desktoptheme/" + themeRoot + '/', false)).exists()) {
            themeRoot = themeRoot + '1';
        }
    } else {
        themeRoot = m_newThemeName->text().replace(' ',"_").remove(QRegExp("[^A-Za-z0-9_]"));
    }

    //Save theme items
    QFile customSettingsFile;
    bool customSettingsFileOpen = false;

    if (m_themeCustomized || !m_newThemeName->text().isEmpty()) {

        clearCustomized(themeRoot);

        //Copy all files from the base theme
        QString baseSource = dirs.locate("data", "desktoptheme/" + m_baseTheme + "/metadata.desktop");
        baseSource = baseSource.left(baseSource.lastIndexOf('/', -1));
        KIO::CopyJob *copyBaseTheme = KIO::copyAs(QUrl::fromLocalFile(baseSource), QUrl::fromLocalFile(dirs.locateLocal("data", "desktoptheme/" + themeRoot, true)), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(copyBaseTheme, this);

        //Prepare settings file for customized theme
        if (isCustomized(themeRoot)) {
            customSettingsFile.setFileName(dirs.locateLocal("data", "desktoptheme/" + themeRoot + "/settings"));
            customSettingsFileOpen = customSettingsFile.open(QFile::WriteOnly);
            if (customSettingsFileOpen) {
                QTextStream out(&customSettingsFile);
                out << "baseTheme=" + m_baseTheme + "\r\n";;
            }
        }
        QString settingsSource;

        //Copy each item theme file to new theme folder
        QHashIterator<int, int> i(m_itemThemeReplacements);
        while (i.hasNext()) {
            i.next();
            //Get root directory where item should reside (relative to theme root)
            QString itemRoot = "";
            if (m_itemPaths[i.key()].lastIndexOf('/', -1) != -1) {
                itemRoot= m_itemPaths[i.key()].left(m_itemPaths[i.key()].lastIndexOf('/', -1));
            }
            //Setup source and destination
            QString source;
            QString dest;
            if (i.value() != -1) {
                //Source is a theme
                source = "desktoptheme/" + m_themeRoots[i.value()] + '/' + m_itemPaths[i.key()] + '*';
                dest = "desktoptheme/" + themeRoot + '/' + itemRoot + '/';
                settingsSource = m_themeRoots[i.value()];
            } else {
               //Source is a file
                source = m_itemFileReplacements[i.key()];
                dest = "desktoptheme/" + themeRoot + '/' + itemRoot + '/';
                settingsSource = m_itemFileReplacements[i.key()];
            }


            //Delete item files at destination before copying (possibly there from base theme copy)
            const QStringList deleteFiles = dirs.findAllResources("data", "desktoptheme/" + themeRoot + '/' + m_itemPaths[i.key()] + '*',
                                            KStandardDirs::NoDuplicates);
            for (int j = 0; j < deleteFiles.size(); ++j) {
                KIO::DeleteJob *dj = KIO::del(QUrl::fromLocalFile(deleteFiles.at(j)), KIO::HideProgressInfo);
                KIO::NetAccess::synchronousRun(dj, this);
            }

            //Copy item(s)
            dest = dirs.locateLocal("data", dest, true);
            QStringList copyFiles;
            if (i.value() != -1) {
                copyFiles = dirs.findAllResources("data", source, KStandardDirs::NoDuplicates); //copy from theme
            } else {
                copyFiles << source; //copy from file
            }
            for (int j = 0; j < copyFiles.size(); ++j) {
                KIO::CopyJob *cj = KIO::copy(QUrl::fromLocalFile(copyFiles.at(j)), QUrl::fromLocalFile(dest), KIO::HideProgressInfo);
                KIO::NetAccess::synchronousRun(cj, this);
            }

            //Record settings file
            if (customSettingsFileOpen) {
                QTextStream out(&customSettingsFile);
                out << m_items.key(i.key()) + "=" + settingsSource +"\r\n";
            }
        }
        if (customSettingsFileOpen) customSettingsFile.close();

        // Create new theme FDO desktop file
        QFile::remove(dirs.locateLocal("data", "desktoptheme/" + themeRoot + "/metadata.desktop", false));
        KDesktopFile df(dirs.locateLocal("data", "desktoptheme/" + themeRoot + "/metadata.desktop"));
        KConfigGroup cg = df.desktopGroup();
        if (isCustomized(themeRoot)) {
            cg.writeEntry("Name",i18n("(Customized)"));
            cg.writeEntry("Comment", i18n("User customized theme"));
            cg.writeEntry("X-KDE-PluginInfo-Name", themeRoot);
        } else {
            cg.writeEntry("Name", m_newThemeName->text());
            cg.writeEntry("Comment", m_newThemeDescription->text());
            cg.writeEntry("X-KDE-PluginInfo-Author", m_newThemeAuthor->text());
            cg.writeEntry("X-KDE-PluginInfo-Version", m_newThemeVersion->text());
        }
        cg.sync();

        m_themeCustomized = false;
    }

    m_themeModel->reload();
    if (m_themeModel->indexOf(themeRoot).isValid()) {
        m_theme->setCurrentIndex(m_themeModel->indexOf(themeRoot));
        QString themeName = m_theme->currentIndex().data(Qt::DisplayRole).toString();
        setDesktopTheme(themeRoot);
    }
    resetThemeDetails();
}

void DesktopThemeDetails::removeTheme()
{
    bool removeTheme = true;
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig("plasmarc"), "Theme");
    QString activeTheme = cfg.readEntry("name", "default");
    const QString theme = m_theme->currentIndex().data(ThemeModel::PackageNameRole).toString();
    const QString themeName = m_theme->currentIndex().data(Qt::DisplayRole).toString();


    if (m_themeCustomized) {
        if(KMessageBox::questionYesNo(this, i18n("Theme items have been changed.  Do you still wish remove the \"%1\" theme?", themeName), i18n("Remove Desktop Theme")) == KMessageBox::No) {
            removeTheme = false;
        }
    } else {
        if (theme == "default") {
            KMessageBox::information(this, i18n("Removal of the default desktop theme is not allowed."), i18n("Remove Desktop Theme"));
            removeTheme = false;
        } else {
            if(KMessageBox::questionYesNo(this, i18n("Are you sure you wish remove the \"%1\" theme?", themeName), i18n("Remove Desktop Theme")) == KMessageBox::No) {
                removeTheme = false;
            }
        }

    }
    KStandardDirs dirs;
    if (removeTheme) {
        if (theme == activeTheme) {
            setDesktopTheme("default");
            activeTheme = "default";
        }
        if (QDir(dirs.locateLocal("data", "desktoptheme/" + theme, false)).exists()) {
            KIO::DeleteJob *deleteTheme = KIO::del(QUrl::fromLocalFile(dirs.locateLocal("data", "desktoptheme/" + theme, false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(deleteTheme, this);
        }
    }
    m_themeModel->reload();
    reloadConfig();
    m_theme->setCurrentIndex(m_themeModel->indexOf(activeTheme));
}

void DesktopThemeDetails::exportTheme()
{
    const QString theme = m_theme->currentIndex().data(ThemeModel::PackageNameRole).toString();

    if (m_themeCustomized ||
        (isCustomized(theme) && m_newThemeName->text() == "")) {
        KMessageBox::information(this, i18n("Please apply theme item changes (with a new theme name) before attempting to export theme."), i18n("Export Desktop Theme"));
    } else {
        QString themeStoragePath = theme;

        const QString themePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "desktoptheme/" + themeStoragePath + "/metadata.desktop");
        if (!themePath.isEmpty()){
            QString expFileName = QFileDialog::getSaveFileName(this, i18n("Export theme to file"), QString(), i18n("Archive (*.zip)"));
            if (!expFileName.endsWith(".zip"))
                expFileName = expFileName + ".zip";
            if (!expFileName.isEmpty()) {
                KUrl path(themePath);
                KZip expFile(expFileName);
                expFile.open(QIODevice::WriteOnly);
                expFile.addLocalDirectory(path.directory (), themeStoragePath);
                expFile.close();
            }
        }
    }
}

void DesktopThemeDetails::loadThemeItems()
{
    // Set up items, item paths and  icons
    m_items.clear(); // clear theme items
    m_itemPaths.clear(); // clear theme item paths
    m_itemIcons.clear();
    for (int i = 0; themeCollectionName[i].m_type; ++i) {
        m_items[themeCollectionName[i].m_type] = i;
        m_itemPaths[i] = themeCollectionName[i].m_themeItemPath;
        m_itemIcons[i] = themeCollectionName[i].m_iconName;
    }

    // Get installed themes
    m_themes.clear(); // clear installed theme list
    m_themeRoots.clear(); // clear installed theme root paths
    KStandardDirs dirs;
    QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop",
                                               KStandardDirs::NoDuplicates);
    themes.sort();
    int j=0;
    for (int i = 0; i < themes.size(); ++i) {
        QString theme = themes.at(i);
        int themeSepIndex = theme.lastIndexOf('/', -1);
        QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
        QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);
        KDesktopFile df(theme);
        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }

        if (!isCustomized(packageName) && (m_themeRoots.key(packageName, -1) == -1)) {
            m_themes[name] = j;
            m_themeRoots[j] = packageName;
            ++j;
        }
    }

    // Set up default item replacements
    m_itemThemeReplacements.clear();
    m_itemFileReplacements.clear();
    QString currentTheme = m_theme->currentIndex().data(ThemeModel::PackageNameRole).toString();

    if (!isCustomized(currentTheme)) {
        // Set default replacements to current theme
        QHashIterator<QString, int> i(m_items);
        while (i.hasNext()) {
            i.next();
            m_itemThemeReplacements[i.value()] = m_themeRoots.key(currentTheme);
        }
        m_baseTheme = currentTheme;
    } else {
        // Set default replacements to customized theme settings
        QFile customSettingsFile(dirs.locateLocal("data", "desktoptheme/" + currentTheme +"/settings"));
        if (customSettingsFile.open(QFile::ReadOnly)) {
            QTextStream in(&customSettingsFile);
            QString line;
            QStringList settingsPair;
            while (!in.atEnd()) {
                line = in.readLine();
                settingsPair = line.split('=');
                if (settingsPair.at(0) == "baseTheme") {
                    m_baseTheme = settingsPair.at(1);
                } else {
                    if (m_themeRoots.key(settingsPair.at(1), -1) != -1) { // theme for current item exists
                        m_itemThemeReplacements[m_items[settingsPair.at(0)]] = m_themeRoots.key(settingsPair.at(1));
                    } else if (QFile::exists(settingsPair.at(1))) {
                        m_itemThemeReplacements[m_items[settingsPair.at(0)]] = -1;
                        m_itemFileReplacements[m_items[settingsPair.at(0)]] = settingsPair.at(1);
                    }
                }
            }
            customSettingsFile.close();
        }
    }

    // Build displayed list of theme items
    m_themeItemList->setRowCount(m_items.size());
    m_themeItemList->setColumnCount(2);
    m_themeItemList->setHorizontalHeaderLabels(QStringList()<< i18n("Theme Item")<<i18n("Source"));
    QString displayedItem;
    QHashIterator<QString, int> i(m_items);
    while (i.hasNext()) {
        i.next();
        displayedItem = displayedItemText(i.value());
        m_themeItemList->setItem(i.value(), 0, new QTableWidgetItem(displayedItem));
        m_themeItemList->item(i.value(),0)->setIcon(QIcon::fromTheme(m_itemIcons[i.value()]));
        m_themeItemList->setCellWidget(i.value(), 1, new QComboBox());
        updateReplaceItemList(i.value());
        m_themeItemList->resizeColumnToContents(1);
    }
    m_themeItemList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_themeItemList->verticalHeader()->hide();
    m_themeItemList->horizontalHeader()->setStretchLastSection(true);
    m_themeItemList->horizontalHeader()->setMinimumSectionSize(120);
    m_themeItemList->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);;
    m_themeItemList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);;
    m_themeItemList->setCurrentCell(0, 1);
}

void DesktopThemeDetails::updateReplaceItemList(const int& item)
{
    QString currentTheme = m_theme->currentIndex().data(ThemeModel::PackageNameRole).toString();


    // Repopulate combobox droplist
    QComboBox *itemComboBox = static_cast<QComboBox*>(m_themeItemList->cellWidget(item,1));
    disconnect(itemComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DesktopThemeDetails::replacementItemChanged);
    itemComboBox->clear();
    for (int i = 0; i < m_themes.size(); ++i) {
       QString displayedDropListItem = i18n("%1 %2", m_themes.key(i), displayedItemText(item));
       itemComboBox->addItem(displayedDropListItem);
    }
    itemComboBox->addItem(i18n("File..."));

    // Set combobox value to current replacement
    if (m_itemThemeReplacements[item] != -1) {
        itemComboBox->setCurrentIndex(m_itemThemeReplacements[item]);
    } else {
        itemComboBox->addItem(m_itemFileReplacements[item]);
        itemComboBox->setCurrentIndex(itemComboBox->findText(m_itemFileReplacements[item]));
    }
    connect(itemComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DesktopThemeDetails::replacementItemChanged);
}

void DesktopThemeDetails::replacementItemChanged()
{
    //Check items to see if theme has been customized
    m_themeCustomized = true;
    QHashIterator<QString, int> i(m_items);
    while (i.hasNext()) {
        i.next();
        QComboBox *itemComboBox = static_cast<QComboBox*>(m_themeItemList->cellWidget(i.value(), 1));
        int replacement = itemComboBox->currentIndex();
        if (replacement <= (m_themes.size() - 1)) {
            // Item replacement source is a theme
            m_itemThemeReplacements[i.value()] = itemComboBox->currentIndex();
        } else if (replacement > (m_themes.size() - 1)) {
            // Item replacement source is a file
            if (itemComboBox->currentText() == i18n("File...")) {
                //Get the filename for the replacement item
                QString translated_key = i18nc("plasma name", qPrintable( i.key() ) );
                QString fileReplacement = QFileDialog::getOpenFileName(this, i18n("Select File to Use for %1",translated_key));
                if (!fileReplacement.isEmpty()) {
                    m_itemFileReplacements[i.value()] = fileReplacement;
                    int index = itemComboBox->findText(fileReplacement);
                    if (index == -1) itemComboBox->addItem(fileReplacement);
                    disconnect(itemComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DesktopThemeDetails::replacementItemChanged);
                    itemComboBox->setCurrentIndex(itemComboBox->findText(fileReplacement));
                    connect(itemComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DesktopThemeDetails::replacementItemChanged);
                    m_itemThemeReplacements[i.value()] = -1; //source is not a theme
                    m_itemFileReplacements[i.value()] = itemComboBox->currentText();
                } else {
                    // Reset combobox to previous value if no file is selected
                    if (m_itemThemeReplacements[i.value()] != -1) {
                        itemComboBox->setCurrentIndex(m_itemThemeReplacements[i.value()]);
                    } else {
                        itemComboBox->setCurrentIndex(itemComboBox->findText(m_itemFileReplacements[i.value()]));
                    }
                    m_themeCustomized = false;
                }
            } else {
                m_itemThemeReplacements[i.value()] = -1; //source is not a theme
                m_itemFileReplacements[i.value()] = itemComboBox->currentText();
            }
        }
    }

    if (m_themeCustomized) emit changed();
}

void DesktopThemeDetails::newThemeInfoChanged()
{
    emit changed();
}

void DesktopThemeDetails::resetThemeDetails()
{
    QString theme = m_theme->currentIndex().data(ThemeModel::PackageNameRole).toString();

    m_themeInfoName->setText(m_theme->currentIndex().data(Qt::DisplayRole).toString());
    m_themeInfoDescription->setText(m_theme->currentIndex().data(ThemeModel::PackageDescriptionRole).toString());
    QString author = m_theme->currentIndex().data(ThemeModel::PackageAuthorRole).toString();

    if (!author.isEmpty()) {
        m_themeInfoAuthor->setText(i18n(" Author: %1",author));
    } else {
        m_themeInfoAuthor->setText("");
    }
    QString version = m_theme->currentIndex().data(ThemeModel::PackageVersionRole).toString();
    if (!version.isEmpty()) {
       m_themeInfoVersion->setText(i18n("Version: %1",version));
    } else {
       m_themeInfoVersion->setText("");
    }

    loadThemeItems();

    m_newThemeName->clear();
    m_newThemeAuthor->clear();
    m_newThemeVersion->clear();
    m_newThemeDescription->clear();
    m_themeCustomized = false;
 }

void DesktopThemeDetails::toggleAdvancedVisible()
{
    m_newThemeNameLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeName->setVisible(m_enableAdvanced->isChecked());
    m_newThemeAuthor->setVisible(m_enableAdvanced->isChecked());
    m_newThemeAuthorLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeVersion->setVisible(m_enableAdvanced->isChecked());
    m_newThemeVersionLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeDescriptionLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeDescription->setVisible(m_enableAdvanced->isChecked());
    m_exportThemeButton->setVisible(m_enableAdvanced->isChecked());
    m_removeThemeButton->setVisible(m_enableAdvanced->isChecked());
    m_advancedLine->setVisible(m_enableAdvanced->isChecked());
}

bool DesktopThemeDetails::isCustomized(const QString& theme) {
    if (theme == ".customized" || theme == ".customized1") {
        return true;
    } else {
        return false;
    }
}

void DesktopThemeDetails::clearCustomized(const QString& themeRoot) {
    KStandardDirs dirs;

    if ((isCustomized(themeRoot))) {
        // Remove both possible unnamed customized directories
        if (QDir(dirs.locateLocal("data", "desktoptheme/.customized", false)).exists()) {
            KIO::DeleteJob *clearCustom = KIO::del(QUrl::fromLocalFile(dirs.locateLocal("data", "desktoptheme/.customized", false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(clearCustom, this);
        }
        if (QDir(dirs.locateLocal("data", "desktoptheme/.customized1", false)).exists()) {
            KIO::DeleteJob *clearCustom1 = KIO::del(QUrl::fromLocalFile(dirs.locateLocal("data", "desktoptheme/.customized1", false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(clearCustom1, this);
        }
    } else {
        if (QDir(dirs.locateLocal("data", "desktoptheme/" + themeRoot, false)).exists()) {
            KIO::DeleteJob *clearCustom = KIO::del(QUrl::fromLocalFile(dirs.locateLocal("data", "desktoptheme/" + themeRoot, false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(clearCustom, this);
        }
    }
}

QString DesktopThemeDetails::displayedItemText(int item) {
    QString displayedText = m_items.key(item);
    for (int i = 0; themeCollectionName[i].m_type; ++i) {
        if (themeCollectionName[i].m_type == m_items.key(item)) {
            displayedText = i18nc("plasma name", themeCollectionName[i].m_displayItemName);
        }
    }
    return displayedText;
}

void DesktopThemeDetails::themeSelectionChanged(const QItemSelection newSelection, const QItemSelection oldSelection)
{
    QString theme = m_theme->currentIndex().data(ThemeModel::PackageNameRole).toString();
    if (theme == "default") {
        m_removeThemeButton->setEnabled(false);
    } else {
        m_removeThemeButton->setEnabled(true);
    }
    resetThemeDetails();
    Q_UNUSED(newSelection);
    Q_UNUSED(oldSelection);
}

void DesktopThemeDetails::setDesktopTheme(QString themeRoot)
{
    KConfig config(QStandardPaths::locate(QStandardPaths::ConfigLocation, "plasmarc"));
    KConfigGroup cg = KConfigGroup(&config, "Theme");
    if (themeRoot == "default") {
        cg.deleteEntry("name");
    } else {
        cg.writeEntry("name", themeRoot);
    }
    cg.sync();
}
