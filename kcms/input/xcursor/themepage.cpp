/*
 * Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config-X11.h>

#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

#include <KGlobalSettings>
#include <KMessageBox>
#include <KUrlRequesterDialog>
#include <KIO/Job>
#include <KIO/DeleteJob>
#include <kio/netaccess.h>
#include <KNewStuff3/KNS3/DownloadDialog>
#include <KLocalizedString>


#include <KTar>

#include <klauncher_iface.h>
#include "../../krdb/krdb.h"

#include <QWidget>
#include <QPushButton>
#include <QDir>
#include <QX11Info>

#include "themepage.h"
#include "themepage.moc"

#include "thememodel.h"
#include "itemdelegate.h"
#include "sortproxymodel.h"
#include "cursortheme.h"

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#ifdef HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
#endif


ThemePage::ThemePage(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    installKnsButton->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
    installButton->setIcon(QIcon::fromTheme("document-import"));
    removeButton->setIcon(QIcon::fromTheme("edit-delete"));

    model = new CursorThemeModel(this);

    proxy = new SortProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseSensitive);
    proxy->sort(NameColumn, Qt::AscendingOrder);

    // Get the icon size for QListView
    int size = style()->pixelMetric(QStyle::PM_LargeIconSize);

    view->setModel(proxy);
    view->setItemDelegate(new ItemDelegate(this));
    view->setIconSize(QSize(size, size));
    view->setSelectionMode(QAbstractItemView::SingleSelection);

    // Make sure we find out about selection changes
    connect(view->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectionChanged()));

    // Make sure we find out about size changes
    connect(sizeComboBox,
            SIGNAL(currentIndexChanged(int)),
            SLOT(sizeChanged()));

    // Make sure we find out about user activity
    connect(sizeComboBox,
            SIGNAL(activated(int)),
            SLOT(preferredSizeChanged()));

    // Disable the install button if we can't install new themes to ~/.icons,
    // or Xcursor isn't set up to look for cursor themes there.
    if (!model->searchPaths().contains(QDir::homePath() + "/.icons") || !iconsIsWritable()) {
            installButton->setEnabled(false);
            installKnsButton->setEnabled(false);
    }

    connect(installKnsButton, SIGNAL(clicked()), SLOT(getNewClicked()));
    connect(installButton, SIGNAL(clicked()), SLOT(installClicked()));
    connect(removeButton,  SIGNAL(clicked()), SLOT(removeClicked()));
}


ThemePage::~ThemePage()
{
}


bool ThemePage::iconsIsWritable() const
{
    const QFileInfo icons = QFileInfo(QDir::homePath() + "/.icons");
    const QFileInfo home  = QFileInfo(QDir::homePath());

    return ((icons.exists() && icons.isDir() && icons.isWritable()) ||
            (!icons.exists() && home.isWritable()));
}


void ThemePage::updateSizeComboBox()
{
    // clear the combo box
    sizeComboBox->clear();

    // refill the combo box and adopt its icon size
    QModelIndex selected = selectedIndex();
    int maxIconWidth = 0;
    int maxIconHeight = 0;
    if (selected.isValid())
    {
        const CursorTheme *theme = proxy->theme(selected);
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
            sizeComboBox->addItem(
                QIcon(m_pixmap),
                i18nc("@item:inlistbox size", "resolution dependent"),
                0);
            comboBoxList << 0;
            foreach (i, sizes)
            {
                m_pixmap = theme->createIcon(i);
                if (m_pixmap.width() > maxIconWidth)
                    maxIconWidth = m_pixmap.width();
                if (m_pixmap.height() > maxIconHeight)
                    maxIconHeight = m_pixmap.height();
                sizeComboBox->addItem(QIcon(m_pixmap), QString::number(i), i);
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
            sizeComboBox->setCurrentIndex(selectItem);
        };
    };
    sizeComboBox->setIconSize(QSize(maxIconWidth, maxIconHeight));

    // enable or disable the combobox
    KConfig c("kcminputrc");
    KConfigGroup cg(&c, "Mouse");
    if (cg.isEntryImmutable("cursorSize")) {
        sizeComboBox->setEnabled(false);
        sizeLabel->setEnabled(false);
    } else {
        sizeComboBox->setEnabled(sizeComboBox->count() > 0);
        sizeLabel->setEnabled(sizeComboBox->count() > 0);
    };
}


int ThemePage::selectedSize() const
{
  if (sizeComboBox->isEnabled() && sizeComboBox->currentIndex() >= 0)
      return sizeComboBox->itemData(sizeComboBox->currentIndex(), Qt::UserRole).toInt();
  return 0;
}


void ThemePage::updatePreview()
{
    QModelIndex selected = selectedIndex();

    if (selected.isValid()) {
        const CursorTheme *theme = proxy->theme(selected);
        preview->setTheme(theme, selectedSize());
        removeButton->setEnabled(theme->isWritable());
    } else {
        preview->setTheme(NULL, 0);
        removeButton->setEnabled(false);
    };
}


bool ThemePage::applyTheme(const CursorTheme *theme, const int size)
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


void ThemePage::save()
{
    const CursorTheme *theme = selectedIndex().isValid() ? proxy->theme(selectedIndex()) : NULL;
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
        KMessageBox::information(this,
                                 i18n("You have to restart KDE for these changes to take effect."),
                                 i18n("Cursor Settings Changed"), "CursorSettingsChanged");
    }

    appliedIndex = selectedIndex();
    appliedSize = size;
}


void ThemePage::load()
{
    view->selectionModel()->clear();
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
        appliedIndex = proxy->findIndex(currentTheme);
    else
        appliedIndex = proxy->defaultIndex();

    // Disable the listview and the buttons if we're in kiosk mode
    if (cg.isEntryImmutable("cursorTheme"))
    {
        view->setEnabled(false);
        installButton->setEnabled(false);
        removeButton->setEnabled(false);
    }

    // Load cursor size
    int size = cg.readEntry("cursorSize", 0);
    if (size <= 0)
        preferredSize = 0;
    else
        preferredSize = size;
    updateSizeComboBox(); // This handles also the kiosk mode

    appliedSize = size;

    const CursorTheme *theme = proxy->theme(appliedIndex);

    if (appliedIndex.isValid())
    {
        // Select the current theme
        view->setCurrentIndex(appliedIndex);
        view->scrollTo(appliedIndex, QListView::PositionAtCenter);

        // Update the preview widget as well
        preview->setTheme(theme, size);
    }

    if (!theme || !theme->isWritable())
        removeButton->setEnabled(false);
}


void ThemePage::defaults()
{
    view->selectionModel()->clear();
    QModelIndex defaultIndex = proxy->findIndex("breeze_cursors");
    view->setCurrentIndex(defaultIndex);
    preferredSize = 0;
    updateSizeComboBox();
}


void ThemePage::selectionChanged()
{
    updateSizeComboBox();
    updatePreview();

    emit changed(appliedIndex != selectedIndex());
}

QModelIndex ThemePage::selectedIndex() const
{
    QModelIndexList selection = view->selectionModel()->selectedIndexes();
    if (!selection.isEmpty()) {
        return (selection.at(0));
    }
    return QModelIndex();
}

void ThemePage::sizeChanged()
{
    updatePreview();
    emit changed(selectedSize() != appliedSize);
}

void ThemePage::preferredSizeChanged()
{
    int index = sizeComboBox->currentIndex();
    if (index >= 0)
        preferredSize = sizeComboBox->itemData(index, Qt::UserRole).toInt();
    else
        preferredSize = 0;
}

void ThemePage::getNewClicked()
{
    KNS3::DownloadDialog dialog("xcursor.knsrc", this);
    if (dialog.exec()) {
        KNS3::Entry::List list = dialog.changedEntries();
        if (list.count() > 0)
            model->refreshList();
    }
}

void ThemePage::installClicked()
{
    // Get the URL for the theme we're going to install
    QUrl url = KUrlRequesterDialog::getUrl(QUrl(), this, i18n("Drag or Type Theme URL"));

    if (url.isEmpty())
        return;

    QString tempFile;
    if (!KIO::NetAccess::download(url, tempFile, this))
    {
        QString text;

        if (url.isLocalFile())
            text = i18n("Unable to find the cursor theme archive %1.",
                        url.toDisplayString());
        else
            text = i18n("Unable to download the cursor theme archive; "
                        "please check that the address %1 is correct.",
                        url.toDisplayString());

        KMessageBox::sorry(this, text);
        return;
    }

    if (!installThemes(tempFile))
        KMessageBox::error(this, i18n("The file %1 does not appear to be a valid "
                                      "cursor theme archive.", url.fileName()));

    KIO::NetAccess::removeTempFile(tempFile);
}


void ThemePage::removeClicked()
{
    // We don't have to check if the current index is valid, since
    // the remove button will be disabled when there's no selection.
    const CursorTheme *theme = proxy->theme(selectedIndex());

    // Don't let the user delete the currently configured theme
    if (selectedIndex() == appliedIndex) {
        KMessageBox::sorry(this, i18n("<qt>You cannot delete the theme you are currently "
                "using.<br />You have to switch to another theme first.</qt>"));
        return;
    }

    // Get confirmation from the user
    QString question = i18n("<qt>Are you sure you want to remove the "
            "<i>%1</i> cursor theme?<br />"
            "This will delete all the files installed by this theme.</qt>",
            theme->title());

    int answer = KMessageBox::warningContinueCancel(this, question,
            i18n("Confirmation"), KStandardGuiItem::del());

    if (answer != KMessageBox::Continue)
        return;

    // Delete the theme from the harddrive
    KIO::del(QUrl::fromLocalFile(theme->path())); // async

    // Remove the theme from the model
    proxy->removeTheme(selectedIndex());

    // TODO:
    //  Since it's possible to substitute cursors in a system theme by adding a local
    //  theme with the same name, we shouldn't remove the theme from the list if it's
    //  still available elsewhere. We could add a
    //  bool CursorThemeModel::tryAddTheme(const QString &name), and call that, but
    //  since KIO::del() is an asynchronos operation, the theme we're deleting will be
    //  readded to the list again before KIO has removed it.
}


bool ThemePage::installThemes(const QString &file)
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

            int answer = KMessageBox::warningContinueCancel(this, question,
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
        model->addTheme(dest);
    }

    archive.close();
    return true;
}

