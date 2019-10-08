/*
 *  Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *  Copyright © 2019 Benjamin Port <benjamin.port@enioka.com>
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

#include <config-X11.h>

#include "kcmcursortheme.h"

#include "xcursor/thememodel.h"
#include "xcursor/sortproxymodel.h"
#include "xcursor/cursortheme.h"
#include "xcursor/previewwidget.h"
#include "../krdb/krdb.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlRequesterDialog>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KTar>
#include <KGlobalSettings>

#include <KNewStuff3/KNS3/DownloadDialog>

#include <QX11Info>
#include <QStandardItemModel>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#include "cursorthemesettings.h"
#include <klauncher_iface.h>

#ifdef HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
#endif

K_PLUGIN_FACTORY_WITH_JSON(CursorThemeConfigFactory, "kcm_cursortheme.json", registerPlugin<CursorThemeConfig>();)

CursorThemeConfig::CursorThemeConfig(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args),
      m_settings(new CursorThemeSettings),
      m_appliedSize(0),
      m_preferredSize(0),
      m_selectedThemeRow(-1),
      m_selectedSizeRow(-1),
      m_originalSelectedThemeRow(-1),
      m_canInstall(true),
      m_canResize(true),
      m_canConfigure(true)
{
    // Unfortunately doesn't generate a ctor taking the parent as parameter
    m_settings->setParent(this);
    qmlRegisterType<PreviewWidget>("org.kde.private.kcm_cursortheme", 1, 0, "PreviewWidget");
    qmlRegisterType<SortProxyModel>();

    KAboutData* aboutData = new KAboutData(QStringLiteral("kcm_cursortheme"), i18n("Cursors"),
        QStringLiteral("1.0"), QString(), KAboutLicense::GPL, i18n("(c) 2003-2007 Fredrik Höglund"));
    aboutData->addAuthor(i18n("Fredrik Höglund"));
    aboutData->addAuthor(i18n("Marco Martin"));
    setAboutData(aboutData);

    m_themeModel = new CursorThemeModel(this);

    m_themeProxyModel = new SortProxyModel(this);
    m_themeProxyModel->setSourceModel(m_themeModel);
    m_themeProxyModel->setFilterCaseSensitivity(Qt::CaseSensitive);
    m_themeProxyModel->sort(NameColumn, Qt::AscendingOrder);

    m_sizesModel = new QStandardItemModel(this);

    // Disable the install button if we can't install new themes to ~/.icons,
    // or Xcursor isn't set up to look for cursor themes there.
    if (!m_themeModel->searchPaths().contains(QDir::homePath() + "/.icons") || !iconsIsWritable()) {
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
    updateSizeComboBox();

    setNeedsSave(m_originalSelectedThemeRow != m_selectedThemeRow || m_originalPreferredSize != m_preferredSize);
}

int CursorThemeConfig::selectedThemeRow() const
{
    return m_selectedThemeRow;
}

void CursorThemeConfig::setSelectedSizeRow(int row)
{
    Q_ASSERT (row < m_sizesModel->rowCount() && row >= 0);

    // we don't return early if m_selectedSizeRow == row as this is called after the model is changed
    m_selectedSizeRow = row;
    emit selectedSizeRowChanged();

    int size = m_sizesModel->item(row)->data().toInt();

    m_preferredSize = size;
    setNeedsSave(m_originalSelectedThemeRow != m_selectedThemeRow || m_originalPreferredSize != m_preferredSize);
}

int CursorThemeConfig::selectedSizeRow() const
{
    return m_selectedSizeRow;
}

bool CursorThemeConfig::downloadingFile() const
{
    return m_tempCopyJob;
}



QAbstractItemModel *CursorThemeConfig::cursorsModel()
{
    return m_themeProxyModel;
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
    QModelIndex selected = themeSelectedIndex();
    int maxIconWidth = 0;
    int maxIconHeight = 0;
    if (selected.isValid()) {
        const CursorTheme *theme = m_themeProxyModel->theme(selected);
        const QList<int> sizes = theme->availableSizes();
        QIcon m_icon;
        // only refill the combobox if there is more that 1 size
        if (sizes.size() > 1) {
            int i;
            QList<int> comboBoxList;
            QPixmap m_pixmap;

            // insert the items
            m_pixmap = theme->createIcon(0);
            if (m_pixmap.width() > maxIconWidth) {
                maxIconWidth = m_pixmap.width();
            }
            if (m_pixmap.height() > maxIconHeight) {
                maxIconHeight = m_pixmap.height();
            }
            QStandardItem *item = new QStandardItem(QIcon(m_pixmap),
                i18nc("@item:inlistbox size", "Resolution dependent"));
            item->setData(0);
            m_sizesModel->appendRow(item);
            comboBoxList << 0;
            foreach (i, sizes) {
                m_pixmap = theme->createIcon(i);
                if (m_pixmap.width() > maxIconWidth) {
                    maxIconWidth = m_pixmap.width();
                }
                if (m_pixmap.height() > maxIconHeight) {
                    maxIconHeight = m_pixmap.height();
                }
                QStandardItem *item = new QStandardItem(QIcon(m_pixmap), QString::number(i));
                item->setData(i);
                m_sizesModel->appendRow(item);
                comboBoxList << i;
            }

            // select an item
            int selectItem = comboBoxList.indexOf(m_preferredSize);
            // m_preferredSize not available for this theme
            if (selectItem < 0) {
                /* Search the value next to m_preferredSize. The first entry (0)
                   is ignored. (If m_preferredSize would have been 0, then we
                   would had found it yet. As m_preferredSize is not 0, we won't
                   default to "automatic size".)*/
                int j;
                int distance;
                int smallestDistance;
                selectItem = 1;
                j = comboBoxList.value(selectItem);
                smallestDistance = j < m_preferredSize ? m_preferredSize - j : j - m_preferredSize;
                for (int i = 2; i < comboBoxList.size(); ++i) {
                    j = comboBoxList.value(i);
                    distance = j < m_preferredSize ? m_preferredSize - j : j - m_preferredSize;
                    if (distance < smallestDistance || (distance == smallestDistance && j > m_preferredSize)) {
                        smallestDistance = distance;
                        selectItem = i;
                    }
                }
            }
            if (selectItem < 0) {
                selectItem = 0;
            }
            setSelectedSizeRow(selectItem);
        }
    }

    // enable or disable the combobox
    if (m_settings->isImmutable("cursorSize")) {
        setCanResize(false);
    } else {
        setCanResize(m_sizesModel->rowCount() > 0);
    }

}

bool CursorThemeConfig::applyTheme(const CursorTheme *theme, const int size)
{
    // Require the Xcursor version that shipped with X11R6.9 or greater, since
    // in previous versions the Xfixes code wasn't enabled due to a bug in the
    // build system (freedesktop bug #975).
#if HAVE_XFIXES && XFIXES_MAJOR >= 2 && XCURSOR_LIB_VERSION >= 10105
    if (!theme) {
        return false;
    }

    if (!CursorTheme::haveXfixes()) {
        return false;
    }

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

    foreach (const QString &name, names) {
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
    const CursorTheme *theme = themeSelectedIndex().isValid() ? m_themeProxyModel->theme(themeSelectedIndex()) : nullptr;

    if (theme) {
        m_settings->setCursorTheme(theme->name());
    }
    m_settings->setCursorSize(m_preferredSize);

    m_settings->save();

    if (!applyTheme(theme, m_preferredSize)) {
        emit showInfoMessage(i18n("You have to restart the Plasma session for these changes to take effect."));
    }

    m_appliedIndex = themeSelectedIndex();
    m_appliedSize = m_preferredSize;
    m_originalSelectedThemeRow = m_selectedThemeRow;
    m_originalPreferredSize = m_preferredSize;
    setNeedsSave(false);
}


void CursorThemeConfig::load()
{
    // Get the name of the theme KDE is configured to use
    QString currentTheme = m_settings->cursorTheme();

    // Find the theme in the listview
    if (!currentTheme.isEmpty()) {
        m_appliedIndex = m_themeProxyModel->findIndex(currentTheme);
    } else {
        m_appliedIndex = m_themeProxyModel->defaultIndex();
    }

    // Disable the listview and the buttons if we're in kiosk mode
    if (m_settings->isImmutable( QStringLiteral( "cursorTheme" ))) {
          setCanConfigure(false);
          setCanInstall(false);
    }

    setSelectedThemeRow(m_appliedIndex.row());
    m_originalSelectedThemeRow = m_selectedThemeRow;

    // Load cursor size
    int size = m_settings->cursorSize();
    if (size <= 0) {
        m_preferredSize = 0;
    } else {
        m_preferredSize = size;
    }
    m_originalPreferredSize = m_preferredSize;
    updateSizeComboBox(); // This handles also the kiosk mode

    m_appliedSize = size;


    setNeedsSave(false);
}


void CursorThemeConfig::defaults()
{
    QModelIndex defaultIndex = m_themeProxyModel->findIndex(m_settings->defaultCursorThemeValue());
    setSelectedThemeRow(defaultIndex.row());
    m_preferredSize = m_settings->defaultCursorSizeValue();
    updateSizeComboBox();
    setNeedsSave(m_originalSelectedThemeRow != m_selectedThemeRow || m_originalPreferredSize != m_preferredSize);
}

QModelIndex CursorThemeConfig::themeSelectedIndex() const
{
    return m_themeProxyModel->index(m_selectedThemeRow, 0);
}

void CursorThemeConfig::getNewClicked()
{
    KNS3::DownloadDialog dialog("xcursor.knsrc", nullptr);
    if (dialog.exec()) {
        KNS3::Entry::List list = dialog.changedEntries();
        if (!list.isEmpty()) {
            for (const KNS3::Entry& entry : list) {
                if (entry.status() == KNS3::Entry::Deleted) {
                    for (const QString& deleted : entry.uninstalledFiles()) {
                        QVector<QStringRef> list = deleted.splitRef(QLatin1Char('/'));
                        if (list.last() == QLatin1Char('*')) {
                            list.takeLast();
                        }
                        QModelIndex idx = m_themeModel->findIndex(list.last().toString());
                        if (idx.isValid()) {
                            m_themeModel->removeTheme(idx);
                        }
                    }
                } else if (entry.status() == KNS3::Entry::Installed) {
                    for (const QString& created : entry.installedFiles()) {
                        QStringList list = created.split(QLatin1Char('/'));
                        if (list.last() == QLatin1Char('*')) {
                            list.takeLast();
                        }
                        // Because we sometimes get some extra slashes in the installed files list
                        list.removeAll({});
                        // Because we'll also get the containing folder, if it was not already there
                        // we need to ignore it.
                        if (list.last() == QLatin1String(".icons")) {
                            continue;
                        }
                        m_themeModel->addTheme(list.join(QLatin1Char('/')));
                    }
                }
            }
        }
    }
}

void CursorThemeConfig::installThemeFromFile(const QUrl &url)
{
    if (url.isLocalFile()) {
        installThemeFile(url.toLocalFile());
        return;
    }

    if (m_tempCopyJob) {
        return;
    }

    m_tempInstallFile.reset(new QTemporaryFile());
    if (!m_tempInstallFile->open()) {
        emit showErrorMessage(i18n("Unable to create a temporary file."));
        m_tempInstallFile.reset();
        return;
    }

    m_tempCopyJob = KIO::file_copy(url,QUrl::fromLocalFile(m_tempInstallFile->fileName()),
                                           -1, KIO::Overwrite);
    m_tempCopyJob->uiDelegate()->setAutoErrorHandlingEnabled(true);
    emit downloadingFileChanged();

    connect(m_tempCopyJob, &KIO::FileCopyJob::result, this, [this, url](KJob *job) {
        if (job->error() != KJob::NoError) {
            emit showErrorMessage(i18n("Unable to download the icon theme archive: %1", job->errorText()));
            return;
        }

        installThemeFile(m_tempInstallFile->fileName());
        m_tempInstallFile.reset();
    });
    connect(m_tempCopyJob, &QObject::destroyed, this, &CursorThemeConfig::downloadingFileChanged);
}

void CursorThemeConfig::installThemeFile(const QString &path)
{
    KTar archive(path);
    archive.open(QIODevice::ReadOnly);

    const KArchiveDirectory *archiveDir = archive.directory();
    QStringList themeDirs;

    // Extract the dir names of the cursor themes in the archive, and
    // append them to themeDirs
    foreach(const QString &name, archiveDir->entries()) {
        const KArchiveEntry *entry = archiveDir->entry(name);
        if (entry->isDirectory() && entry->name().toLower() != "default") {
            const KArchiveDirectory *dir = static_cast<const KArchiveDirectory *>(entry);
            if (dir->entry("index.theme") && dir->entry("cursors")) {
                themeDirs << dir->name();
            }
        }
    }

    if (themeDirs.isEmpty()) {
        emit showErrorMessage(i18n("The file is not a valid icon theme archive."));
        return;
    }

    // The directory we'll install the themes to
    QString destDir = QDir::homePath() + "/.icons/";
    if (!QDir().mkpath(destDir)) {
        emit showErrorMessage(i18n("Failed to create 'icons' folder."));
        return;
    }

    // Process each cursor theme in the archive
    foreach (const QString &dirName, themeDirs) {
        QDir dest(destDir + dirName);
        if (dest.exists()) {
            QString question = i18n("A theme named %1 already exists in your icon "
                    "theme folder. Do you want replace it with this one?", dirName);

            int answer = KMessageBox::warningContinueCancel(nullptr, question,
                                i18n("Overwrite Theme?"),
                                KStandardGuiItem::overwrite());

            if (answer != KMessageBox::Continue) {
                continue;
            }

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
        m_themeModel->addTheme(dest);
    }

    archive.close();

    emit showSuccessMessage(i18n("Theme installed successfully."));

    m_themeModel->refreshList();
}

void CursorThemeConfig::removeTheme(int row)
{
    QModelIndex idx = m_themeProxyModel->index(row, 0);
    if (!idx.isValid()) {
        return;
    }

    const CursorTheme *theme = m_themeProxyModel->theme(idx);

    // Don't let the user delete the currently configured theme
    if (idx == m_appliedIndex) {
        KMessageBox::sorry(nullptr, i18n("<qt>You cannot delete the theme you are currently "
                "using.<br />You have to switch to another theme first.</qt>"));
        return;
    }

    // Get confirmation from the user
    QString question = i18n("<qt>Are you sure you want to remove the "
            "<i>%1</i> cursor theme?<br />"
            "This will delete all the files installed by this theme.</qt>",
            theme->title());

    int answer = KMessageBox::warningContinueCancel(nullptr, question,
            i18n("Confirmation"), KStandardGuiItem::del());

    if (answer != KMessageBox::Continue) {
        return;
    }

    // Delete the theme from the harddrive
    KIO::del(QUrl::fromLocalFile(theme->path())); // async

    // Remove the theme from the model
    m_themeProxyModel->removeTheme(idx);

    // TODO:
    //  Since it's possible to substitute cursors in a system theme by adding a local
    //  theme with the same name, we shouldn't remove the theme from the list if it's
    //  still available elsewhere. We could add a
    //  bool CursorThemeModel::tryAddTheme(const QString &name), and call that, but
    //  since KIO::del() is an asynchronos operation, the theme we're deleting will be
    //  readded to the list again before KIO has removed it.
}

#include "kcmcursortheme.moc"
