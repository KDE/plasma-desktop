/*
 *  Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kcmcursortheme.h"

#include "xcursor/thememodel.h"
#include "xcursor/itemdelegate.h"
#include "xcursor/sortproxymodel.h"
#include "xcursor/cursortheme.h"
#include "xcursor/previewwidget.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KUrlRequesterDialog>
#include <KIO/Job>
#include <KIO/DeleteJob>
#include <kio/netaccess.h>
#include <KTar>

#include <KNewStuff3/KNS3/DownloadDialog>

#include <QX11Info>
#include <QStandardItemModel>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#ifdef HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
#endif

K_PLUGIN_FACTORY_WITH_JSON(CursorThemeConfigFactory, "kcm_cursortheme.json", registerPlugin<CursorThemeConfig>();)

CursorThemeConfig::CursorThemeConfig(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args),
      m_canInstall(true),
      m_canRemove(true),
      m_canResize(true),
      m_canConfigure(true),
      m_selectedThemeRow(-1)
{
    qmlRegisterType<PreviewWidget>("org.kde.private.kcm_cursortheme", 1, 0, "PreviewWidget");
    qmlRegisterType<SortProxyModel>();

    KAboutData* aboutData = new KAboutData(QStringLiteral("kcm_cursortheme"), i18n("Cursor Theme"),
        QStringLiteral("1.0"), QString(), KAboutLicense::GPL, i18n("(c) 2003-2007 Fredrik Höglund"));
    aboutData->addAuthor(i18n("Fredrik Höglund"));
    aboutData->addAuthor(i18n("Marco Martin"));
    setAboutData(aboutData);

    m_model = new CursorThemeModel(this);

    m_proxyModel = new SortProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseSensitive);
    m_proxyModel->sort(NameColumn, Qt::AscendingOrder);

    m_sizesModel = new QStandardItemModel(this);

    // Disable the install button if we can't install new themes to ~/.icons,
    // or Xcursor isn't set up to look for cursor themes there.
    if (!m_model->searchPaths().contains(QDir::homePath() + "/.icons") || !iconsIsWritable()) {
        setCanInstall(false);
    }
}

CursorThemeConfig::~CursorThemeConfig()
{
    /* */
}

void CursorThemeConfig::setCanInstall(bool can)
{
    if (m_canInstall == can) {
        return;
    }

    m_canInstall = can;
    emit canInstallChanged();
}

bool CursorThemeConfig::canInstall() const
{
    return m_canInstall;
}

void CursorThemeConfig::setCanRemove(bool can)
{
    if (m_canRemove == can) {
        return;
    }

    m_canRemove = can;
    emit canRemoveChanged();
}

bool CursorThemeConfig::canRemove() const
{
    return m_canRemove;
}

void CursorThemeConfig::setCanResize(bool can)
{
    if (m_canResize == can) {
        return;
    }

    m_canResize = can;
    emit canResizeChanged();
}

bool CursorThemeConfig::canResize() const
{
    return m_canResize;
}

void CursorThemeConfig::setCanConfigure(bool can)
{
    if (m_canConfigure == can) {
        return;
    }

    m_canConfigure = can;
    emit canConfigureChanged();
}

bool CursorThemeConfig::canConfigure() const
{
    return m_canConfigure;
}

void CursorThemeConfig::setSelectedThemeRow(int row)
{
    if (m_selectedThemeRow == row) {
        return;
    }

    m_selectedThemeRow = row;
    emit selectedThemeRowChanged();
    setNeedsSave(true);
    updateSizeComboBox();

    QModelIndex selected = selectedIndex();

    if (selected.isValid()) {
        const CursorTheme *theme = m_proxyModel->theme(selected);
        setCanRemove(theme->isWritable());
    } else {
        setCanRemove(false);
    };
}

int CursorThemeConfig::selectedThemeRow() const
{
    return m_selectedThemeRow;
}


QAbstractItemModel *CursorThemeConfig::cursorsModel()
{
    return m_proxyModel;
}

QAbstractItemModel *CursorThemeConfig::sizesModel()
{
    return m_sizesModel;
}

bool CursorThemeConfig::iconsIsWritable() const
{
    const QFileInfo icons = QFileInfo(QDir::homePath() + "/.icons");
    const QFileInfo home  = QFileInfo(QDir::homePath());

    return ((icons.exists() && icons.isDir() && icons.isWritable()) ||
            (!icons.exists() && home.isWritable()));
}


void CursorThemeConfig::updateSizeComboBox()
{

    // clear the combo box
    m_sizesModel->clear();

    // refill the combo box and adopt its icon size
    QModelIndex selected = selectedIndex();
    int maxIconWidth = 0;
    int maxIconHeight = 0;
    if (selected.isValid())
    {
        const CursorTheme *theme = m_proxyModel->theme(selected);
        const QList<int> sizes = theme->availableSizes();
        QIcon m_icon;
        if (sizes.size() > 1)  // only refill the combobox if there is more that 1 size
        {
            int i;
            QList<int> comboBoxList;
            QPixmap m_pixmap;

            // insert the items
            m_pixmap = theme->createIcon(0);
            if (m_pixmap.width() > maxIconWidth)
                maxIconWidth = m_pixmap.width();
            if (m_pixmap.height() > maxIconHeight)
                maxIconHeight = m_pixmap.height();
            QStandardItem *item = new QStandardItem(QIcon(m_pixmap),
                i18nc("@item:inlistbox size", "Resolution dependent"));
            item->setData(0);
            m_sizesModel->appendRow(item);
            comboBoxList << 0;
            foreach (i, sizes)
            {
                m_pixmap = theme->createIcon(i);
                if (m_pixmap.width() > maxIconWidth)
                    maxIconWidth = m_pixmap.width();
                if (m_pixmap.height() > maxIconHeight)
                    maxIconHeight = m_pixmap.height();
                QStandardItem *item = new QStandardItem(QIcon(m_pixmap), QString::number(i));
                item->setData(i);
                m_sizesModel->appendRow(item);
                comboBoxList << i;
            };

            // select an item
            int selectItem = comboBoxList.indexOf(preferredSize);
            if (selectItem < 0)  // preferredSize not available for this theme
            {
                /* Search the value next to preferredSize. The first entry (0)
                   is ignored. (If preferredSize would have been 0, then we
                   would had found it yet. As preferredSize is not 0, we won't
                   default to "automatic size".)*/
                int j;
                int distance;
                int smallestDistance;
                selectItem = 1;
                j = comboBoxList.value(selectItem);
                smallestDistance = j < preferredSize ? preferredSize - j : j - preferredSize;
                for (int i = 2; i < comboBoxList.size(); ++i)
                {
                    j = comboBoxList.value(i);
                    distance = j < preferredSize ? preferredSize - j : j - preferredSize;
                    if (distance < smallestDistance || (distance == smallestDistance && j > preferredSize))
                    {
                        smallestDistance = distance;
                        selectItem = i;
                    };
                }
            };
            //TODO
            //sizeComboBox->setCurrentIndex(selectItem);
        };
    };
    //sizeComboBox->setIconSize(QSize(maxIconWidth, maxIconHeight));

    // enable or disable the combobox
    KConfig c("kcminputrc");
    KConfigGroup cg(&c, "Mouse");
    if (cg.isEntryImmutable("cursorSize")) {
        setCanResize(false);
    } else {
        setCanResize(m_sizesModel->rowCount() > 0);
    };

}


int CursorThemeConfig::selectedSize() const
{
    return 0;
//TODO
#if 0
  if (sizeComboBox->isEnabled() && sizeComboBox->currentIndex() >= 0)
      return sizeComboBox->itemData(sizeComboBox->currentIndex(), Qt::UserRole).toInt();
  return 0;
#endif
}


void CursorThemeConfig::updatePreview()
{
//TODO
#if 0
    QModelIndex selected = selectedIndex();

    if (selected.isValid()) {
        const CursorTheme *theme = m_proxyModel->theme(selected);
        preview->setTheme(theme, selectedSize());
        setCanRemove(theme->isWritable());
    } else {
        preview->setTheme(NULL, 0);
        setCanRemove(false);
    };
#endif
}


bool CursorThemeConfig::applyTheme(const CursorTheme *theme, const int size)
{
    // Require the Xcursor version that shipped with X11R6.9 or greater, since
    // in previous versions the Xfixes code wasn't enabled due to a bug in the
    // build system (freedesktop bug #975).
#if HAVE_XFIXES && XFIXES_MAJOR >= 2 && XCURSOR_LIB_VERSION >= 10105
    if (!theme)
        return false;

    if (!CursorTheme::haveXfixes())
        return false;

    QByteArray themeName = QFile::encodeName(theme->name());

    // Set up the proper launch environment for newly started apps
    OrgKdeKLauncherInterface klauncher(QStringLiteral("org.kde.klauncher5"),
                                       QStringLiteral("/KLauncher"),
                                       QDBusConnection::sessionBus());
    klauncher.setLaunchEnv(QStringLiteral("XCURSOR_THEME"), themeName);

    // Update the Xcursor X resources
    runRdb(0);

    // Notify all applications that the cursor theme has changed
    KGlobalSettings::self()->emitChange(KGlobalSettings::CursorChanged);

    // Reload the standard cursors
    QStringList names;

    // Qt cursors
    names << "left_ptr"       << "up_arrow"      << "cross"      << "wait"
          << "left_ptr_watch" << "ibeam"         << "size_ver"   << "size_hor"
          << "size_bdiag"     << "size_fdiag"    << "size_all"   << "split_v"
          << "split_h"        << "pointing_hand" << "openhand"
          << "closedhand"     << "forbidden"     << "whats_this" << "copy" << "move" << "link";

    // X core cursors
    names << "X_cursor"            << "right_ptr"           << "hand1"
          << "hand2"               << "watch"               << "xterm"
          << "crosshair"           << "left_ptr_watch"      << "center_ptr"
          << "sb_h_double_arrow"   << "sb_v_double_arrow"   << "fleur"
          << "top_left_corner"     << "top_side"            << "top_right_corner"
          << "right_side"          << "bottom_right_corner" << "bottom_side"
          << "bottom_left_corner"  << "left_side"           << "question_arrow"
          << "pirate";

    foreach (const QString &name, names)
    {
        XFixesChangeCursorByName(QX11Info::display(), theme->loadCursor(name, size), QFile::encodeName(name));
    }

    return true;
#else
    Q_UNUSED(theme)
    return false;
#endif
}


void CursorThemeConfig::save()
{
    const CursorTheme *theme = selectedIndex().isValid() ? m_proxyModel->theme(selectedIndex()) : NULL;
    const int size = selectedSize();

    KConfig config("kcminputrc");
    KConfigGroup c(&config, "Mouse");
    if (theme)
    {
        c.writeEntry("cursorTheme", theme->name());
    };
    c.writeEntry("cursorSize", size);
    preferredSize = size;
    c.sync();

    if (!applyTheme(theme, size))
    {
        KMessageBox::information(0,
                                 i18n("You have to restart KDE for these changes to take effect."),
                                 i18n("Cursor Settings Changed"), "CursorSettingsChanged");
    }

    m_appliedIndex = selectedIndex();
    m_appliedSize = size;
    setNeedsSave(false);
}


void CursorThemeConfig::load()
{

    // Get the name of the theme libXcursor currently uses
    QString currentTheme;
    if (QX11Info::isPlatformX11()) {
        currentTheme = XcursorGetTheme(QX11Info::display());
    }

    // Get the name of the theme KDE is configured to use
    KConfig c("kcminputrc");
    KConfigGroup cg(&c, "Mouse");
    currentTheme = cg.readEntry("cursorTheme", currentTheme);

    // Find the theme in the listview
    if (!currentTheme.isEmpty())
        m_appliedIndex = m_proxyModel->findIndex(currentTheme);
    else
        m_appliedIndex = m_proxyModel->defaultIndex();

    // Disable the listview and the buttons if we're in kiosk mode
    if (cg.isEntryImmutable("cursorTheme"))
    {
          setCanConfigure(false);
          setCanInstall(false);
          setCanRemove(false);
    }

    // Load cursor size
    int size = cg.readEntry("cursorSize", 0);
    if (size <= 0)
        preferredSize = 0;
    else
        preferredSize = size;
    updateSizeComboBox(); // This handles also the kiosk mode

    m_appliedSize = size;

    const CursorTheme *theme = m_proxyModel->theme(m_appliedIndex);

    if (m_appliedIndex.isValid())
    {
        // Select the current theme
//TODO
/*        view->setCurrentIndex(m_appliedIndex);
        view->scrollTo(m_appliedIndex, QListView::PositionAtCenter);

        // Update the preview widget as well
        preview->setTheme(theme, size);*/
    }

    if (!theme || !theme->isWritable()) {
        setCanRemove(false);
    }

    setNeedsSave(false);
}


void CursorThemeConfig::defaults()
{
    QModelIndex defaultIndex = m_proxyModel->findIndex("breeze_cursors");
    setSelectedThemeRow(defaultIndex.row());
    preferredSize = 0;
    updateSizeComboBox();
    setNeedsSave(true);
}


void CursorThemeConfig::selectionChanged()
{
    updateSizeComboBox();
    updatePreview();

    setNeedsSave(m_appliedIndex != selectedIndex());
}

QModelIndex CursorThemeConfig::selectedIndex() const
{
    return m_proxyModel->index(m_selectedThemeRow, 0);
}

void CursorThemeConfig::sizeChanged()
{
    updatePreview();
    setNeedsSave(selectedSize() != m_appliedSize);
}

void CursorThemeConfig::preferredSizeChanged()
{
    //TODO
    /*
    int index = sizeComboBox->currentIndex();
    if (index >= 0)
        preferredSize = sizeComboBox->itemData(index, Qt::UserRole).toInt();
    else
        preferredSize = 0;*/
}

void CursorThemeConfig::getNewClicked()
{
    KNS3::DownloadDialog dialog("xcursor.knsrc", 0);
    if (dialog.exec()) {
        KNS3::Entry::List list = dialog.changedEntries();
        if (list.count() > 0)
            m_model->refreshList();
    }
}

void CursorThemeConfig::installClicked()
{
    // Get the URL for the theme we're going to install
    QUrl url = KUrlRequesterDialog::getUrl(QUrl(), 0, i18n("Drag or Type Theme URL"));

    if (url.isEmpty())
        return;

    QString tempFile;
    if (!KIO::NetAccess::download(url, tempFile, 0))
    {
        QString text;

        if (url.isLocalFile())
            text = i18n("Unable to find the cursor theme archive %1.",
                        url.toDisplayString());
        else
            text = i18n("Unable to download the cursor theme archive; "
                        "please check that the address %1 is correct.",
                        url.toDisplayString());

        KMessageBox::sorry(0, text);
        return;
    }

    if (!installThemes(tempFile))
        KMessageBox::error(0, i18n("The file %1 does not appear to be a valid "
                                      "cursor theme archive.", url.fileName()));

    KIO::NetAccess::removeTempFile(tempFile);
}


void CursorThemeConfig::removeClicked()
{
    // We don't have to check if the current index is valid, since
    // the remove button will be disabled when there's no selection.
    const CursorTheme *theme = m_proxyModel->theme(selectedIndex());

    // Don't let the user delete the currently configured theme
    if (selectedIndex() == m_appliedIndex) {
        KMessageBox::sorry(0, i18n("<qt>You cannot delete the theme you are currently "
                "using.<br />You have to switch to another theme first.</qt>"));
        return;
    }

    // Get confirmation from the user
    QString question = i18n("<qt>Are you sure you want to remove the "
            "<i>%1</i> cursor theme?<br />"
            "This will delete all the files installed by this theme.</qt>",
            theme->title());

    int answer = KMessageBox::warningContinueCancel(0, question,
            i18n("Confirmation"), KStandardGuiItem::del());

    if (answer != KMessageBox::Continue)
        return;

    // Delete the theme from the harddrive
    KIO::del(QUrl::fromLocalFile(theme->path())); // async

    // Remove the theme from the model
    m_proxyModel->removeTheme(selectedIndex());

    // TODO:
    //  Since it's possible to substitute cursors in a system theme by adding a local
    //  theme with the same name, we shouldn't remove the theme from the list if it's
    //  still available elsewhere. We could add a
    //  bool CursorThemeModel::tryAddTheme(const QString &name), and call that, but
    //  since KIO::del() is an asynchronos operation, the theme we're deleting will be
    //  readded to the list again before KIO has removed it.
}


bool CursorThemeConfig::installThemes(const QString &file)
{
    KTar archive(file);

    if (!archive.open(QIODevice::ReadOnly))
        return false;

    const KArchiveDirectory *archiveDir = archive.directory();
    QStringList themeDirs;

    // Extract the dir names of the cursor themes in the archive, and
    // append them to themeDirs
    foreach(const QString &name, archiveDir->entries())
    {
        const KArchiveEntry *entry = archiveDir->entry(name);
        if (entry->isDirectory() && entry->name().toLower() != "default")
        {
            const KArchiveDirectory *dir = static_cast<const KArchiveDirectory *>(entry);
            if (dir->entry("index.theme") && dir->entry("cursors"))
                themeDirs << dir->name();
        }
    }

    if (themeDirs.isEmpty())
        return false;

    // The directory we'll install the themes to
    QString destDir = QDir::homePath() + "/.icons/";
    QDir().mkpath(destDir); // Make sure the directory exists

    // Process each cursor theme in the archive
    foreach (const QString &dirName, themeDirs)
    {
        QDir dest(destDir + dirName);
        if (dest.exists())
        {
            QString question = i18n("A theme named %1 already exists in your icon "
                    "theme folder. Do you want replace it with this one?", dirName);

            int answer = KMessageBox::warningContinueCancel(0, question,
                                i18n("Overwrite Theme?"),
                                KStandardGuiItem::overwrite());

            if (answer != KMessageBox::Continue)
                continue;

            // ### If the theme that's being replaced is the current theme, it
            //     will cause cursor inconsistencies in newly started apps.
        }

        // ### Should we check if a theme with the same name exists in a global theme dir?
        //     If that's the case it will effectively replace it, even though the global theme
        //     won't be deleted. Checking for this situation is easy, since the global theme
        //     will be in the listview. Maybe this should never be allowed since it might
        //     result in strange side effects (from the average users point of view). OTOH
        //     a user might want to do this 'upgrade' a global theme.

        const KArchiveDirectory *dir = static_cast<const KArchiveDirectory*>
                        (archiveDir->entry(dirName));
        dir->copyTo(dest.path());
        m_model->addTheme(dest);
    }

    archive.close();
    return true;
}

#include "kcmcursortheme.moc"
