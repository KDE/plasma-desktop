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
#include "scmeditordialog.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QPainter>
#include <QBitmap>
#include <QtDBus/QtDBus>

#include <KAboutData>
#include <KColorButton>
#include <KConfigGroup>
#include <KColorUtils>
#include <KColorScheme>
#include <KMessageBox>
#include <KPluginFactory>
#include <KJobUiDelegate>
#include <KIO/DeleteJob>
#include <KNS3/DownloadDialog>

K_PLUGIN_FACTORY( KolorFactory, registerPlugin<KColorCm>(); )
K_EXPORT_PLUGIN( KolorFactory("kcmcolors") )

KColorCm::KColorCm(QWidget *parent, const QVariantList &)
    : KCModule( parent ),
      m_dontLoadSelectedScheme(false),
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

    m_config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));

    setupUi(this);
    connect(applyToAlien, &QCheckBox::toggled, [=](){ emit changed(true); });
    connect(schemeList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(loadScheme(QListWidgetItem*,QListWidgetItem*)));
    schemeKnsButton->setIcon( QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")) );
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
    const QStringList schemeDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes"), QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString &dir, schemeDirs)
    {
        const QStringList fileNames = QDir(dir).entryList(QStringList()<<QStringLiteral("*.colors"));
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


void KColorCm::loadScheme(KSharedConfigPtr config) // const QString &path)
{
    schemePreview->setPalette(config);
    updateConfig(config);
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
        qDebug() << "dontload";
        return;
    }

    if (currentItem != NULL)
    {

        // load it

        const QString name = currentItem->text();
        m_currentColorScheme = name;
        const QString fileBaseName = currentItem->data(Qt::UserRole).toString();
        if (name == i18nc("Default color scheme", "Default"))
        {
            schemeRemoveButton->setEnabled(false);

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
            loadInternal();
        }
        else
        {
            const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                "color-schemes/" + fileBaseName + ".colors");

            const int permissions = QFile(path).permissions();
            const bool canWrite = (permissions & QFile::WriteUser);
            qDebug() << "checking permissions of " << path;
            schemeRemoveButton->setEnabled(canWrite);

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
        qDebug() << path;
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

        if (config->groupList().contains(QStringLiteral("Color Scheme")))
        {
            KMessageBox::sorry(this,
                i18n("The scheme you have selected appears to be a KDE3 scheme.\n\n"
                     "This is not supported anymore."),
                i18n("Notice"));
            return;
        }
        else
        {
            // load scheme
            loadScheme(config);

            // save it
//FIXME            saveScheme(url.fileName());
        }
    }
}

void KColorCm::on_schemeKnsButton_clicked()
{
    KNS3::DownloadDialog dialog(QStringLiteral("colorschemes.knsrc"), this);
    dialog.exec();
    if ( ! dialog.changedEntries().isEmpty() )
    {
        populateSchemeList();
    }
}

QPixmap KColorCm::createSchemePreviewIcon(const KSharedConfigPtr &config)
{
    const uchar bits1[] = { 0xff, 0xff, 0xff, 0x2c, 0x16, 0x0b };
    const uchar bits2[] = { 0x68, 0x34, 0x1a, 0xff, 0xff, 0xff };
    const QSize bitsSize(24,2);
    const QBitmap b1 = QBitmap::fromData(bitsSize, bits1);
    const QBitmap b2 = QBitmap::fromData(bitsSize, bits2);

    QPixmap pixmap(23, 16);
    pixmap.fill(Qt::black); // ### use some color other than black for borders?

    QPainter p(&pixmap);

    KConfigGroup group(config, "WM");
    // NOTE: keep this in sync with kdelibs/kdeui/kernel/kglobalsettings.cpp
    QColor activeBackground = group.readEntry("activeBackground", QColor(48, 174, 232));
    QColor activeForeground = group.readEntry("activeForeground", QColor(255, 255, 255));
    QColor inactiveBackground = group.readEntry("inactiveBackground", QColor(224, 223, 222));
    QColor inactiveForeground = group.readEntry("inactiveForeground", QColor(75, 71, 67));

    KColorScheme windowScheme(QPalette::Active, KColorScheme::Window, config);
    p.fillRect( 1,  1, 7, 7, windowScheme.background());
    p.fillRect( 2,  2, 5, 2, QBrush(windowScheme.foreground().color(), b1));

    KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button, config);
    p.fillRect( 8,  1, 7, 7, buttonScheme.background());
    p.fillRect( 9,  2, 5, 2, QBrush(buttonScheme.foreground().color(), b1));

    p.fillRect(15,  1, 7, 7, activeBackground);
    p.fillRect(16,  2, 5, 2, QBrush(activeForeground, b1));

    KColorScheme viewScheme(QPalette::Active, KColorScheme::View, config);
    p.fillRect( 1,  8, 7, 7, viewScheme.background());
    p.fillRect( 2, 12, 5, 2, QBrush(viewScheme.foreground().color(), b2));

    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection, config);
    p.fillRect( 8,  8, 7, 7, selectionScheme.background());
    p.fillRect( 9, 12, 5, 2, QBrush(selectionScheme.foreground().color(), b2));

    p.fillRect(15,  8, 7, 7, inactiveBackground);
    p.fillRect(16, 12, 5, 2, QBrush(inactiveForeground, b2));

    p.end();

    return pixmap;
}

void KColorCm::load()
{
    loadInternal();

    // get colorscheme name from global settings
    KConfigGroup group(m_config, "General");
    m_currentColorScheme = group.readEntry("ColorScheme");

    QList<QListWidgetItem*> itemList = schemeList->findItems(m_currentColorScheme, Qt::MatchExactly);
    if(!itemList.isEmpty()) // "Current" is already selected, so don't handle the case that itemList is empty
        schemeList->setCurrentItem(itemList.at(0));

    KConfig cfg(QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals);
    group = KConfigGroup(&cfg, "X11");

    applyToAlien->blockSignals(true); // don't emit SIGNAL(toggled(bool)) which would call SLOT(emitChanged())
    applyToAlien->setChecked(group.readEntry("exportKDEColors", true));
    applyToAlien->blockSignals(false);
}


void KColorCm::loadInternal()
{
    // clean the config, in case we have changed the in-memory kconfig
    m_config->markAsClean();
    m_config->reparseConfiguration();

    // fill in the color scheme list
    populateSchemeList();

    schemePreview->setPalette(m_config);
    emit changed(false);
}

void KColorCm::save()
{
    QIcon icon = createSchemePreviewIcon(m_config);
    schemeList->item(0)->setIcon(icon);

    m_config->sync();

    KConfig cfg(QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals);
    KConfigGroup displayGroup(&cfg, "X11");

    displayGroup.writeEntry("exportKDEColors", applyToAlien->isChecked());

    cfg.sync();
    qDebug() << KRdbExportQtColors << KRdbExportGtkTheme << KRdbExportColors ;
    runRdb(KRdbExportQtColors | KRdbExportGtkTheme | ( applyToAlien->isChecked() ? KRdbExportColors : 0 ) );
    qDebug() << "icic";
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"), QStringLiteral("org.kde.KGlobalSettings"), QStringLiteral("notifyChange") );
    QList<QVariant> args;
    args.append(0);//previous KGlobalSettings::PaletteChanged. This is now private API in khintsettings
    args.append(0);//unused in palette changed but needed for the DBus signature
    message.setArguments(args);
    qDebug() << KConfigGroup(m_config, "General").readEntry("Name");
    QDBusConnection::sessionBus().send(message);
    if (qApp->platformName() == QStringLiteral("xcb")) {
        // Send signal to all kwin instances
        QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig"));
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

    KCModule::defaults();
    emit changed(true);
}

void KColorCm::on_schemeEditButton_clicked()
{
    QListWidgetItem *currentItem = schemeList->currentItem();
    const QString fileBaseName = currentItem->data(Qt::UserRole).toString();
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                "color-schemes/" + fileBaseName + ".colors");
    SchemeEditorDialog* dialog = new SchemeEditorDialog(path, this);
    dialog->setModal(true);
    dialog->show();
}

void KColorCm::updateConfig(KSharedConfigPtr config)
{
    // store colorscheme name in global settings
    KConfigGroup groupOut(m_config, "General");
    groupOut.writeEntry("ColorScheme", m_currentColorScheme);

    QStringList colorItemList;
    colorItemList << "BackgroundNormal"
                  << "BackgroundAlternate"
                  << "ForegroundNormal"
                  << "ForegroundInactive"
                  << "ForegroundActive"
                  << "ForegroundLink"
                  << "ForegroundVisited"
                  << "ForegroundNegative"
                  << "ForegroundNeutral"
                  << "ForegroundPositive"
                  << "DecorationFocus"
                  << "DecorationHover";

    QStringList colorSetGroupList;
    colorSetGroupList << "Colors:Window"
                      << "Colors:Button"
                      << "Colors:Selection"
                      << "Colors:Tooltip"
                      << "Colors:View";
    for (const QString &colorSetGroup : colorSetGroupList)
    {
        KConfigGroup groupOut(m_config, colorSetGroup);
        KConfigGroup groupTheme(config, colorSetGroup);

        for (const QString &coloritem : colorItemList)
        {
            groupOut.writeEntry(coloritem, groupTheme.readEntry(coloritem));
        }
    }

    KConfigGroup groupWMTheme(config, "WM");
    KConfigGroup groupWMOut(m_config, "WM");

    QStringList colorItemListWM;
    colorItemListWM << "activeBackground"
                    << "activeForeground"
                    << "inactiveBackground"
                    << "inactiveForeground"
                    << "activeBlend"
                    << "inactiveBlend";

    for (const QString &coloritem : colorItemListWM)
    {
            groupWMOut.writeEntry(coloritem, groupWMTheme.readEntry(coloritem));
    }


    QStringList groupNameList;
    groupNameList << "ColorEffects:Inactive" << "ColorEffects:Disabled";

    QStringList effectList;
    effectList << "IntensityEffect"
               << "IntensityAmount"
               << "ColorEffect"
               << "ColorAmount"
               << "Color"
               << "ContrastEffect"
               << "ContrastAmount";

    for (const QString &groupName : groupNameList)
    {

        KConfigGroup groupEffectOut(m_config, groupName);
        KConfigGroup groupEffectTheme(config, groupName);

        for (const QString &effect : effectList) {
            groupEffectOut.writeEntry(effect, groupEffectTheme.readEntry("effect"));

        }
    }
}
#include "colorscm.moc"
