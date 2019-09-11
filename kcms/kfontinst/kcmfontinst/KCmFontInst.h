#ifndef __KCM_FONT_INST_H__
#define __KCM_FONT_INST_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "GroupList.h"
#include "JobRunner.h"
#include <QSet>
#include <KCModule>
#include <QUrl>
#include <KConfig>
#include <KIO/Job>

class QPushButton;
class QProgressDialog;
class QTemporaryDir;
class QAction;
class QLabel;
class QMenu;
class QProcess;
class QSplitter;

namespace KFI
{

class CFontFilter;
class CFontList;
class CFontPreview;
class CUpdateDialog;
class CFontListView;
class CProgressBar;
class CPreviewListView;

class CKCmFontInst : public KCModule
{
    Q_OBJECT

    public:

    explicit CKCmFontInst(QWidget *parent=nullptr, const QVariantList &list=QVariantList());
    ~CKCmFontInst() override;

    public Q_SLOTS:

    QString quickHelp() const override;
    void    previewMenu(const QPoint &pos);
    void    splitterMoved();
    void    fontsSelected(const QModelIndexList &list);
    void    groupSelected(const QModelIndex &index);
    void    addFonts();
    void    deleteFonts();
    void    moveFonts();
    void    zipGroup();
    void    enableFonts();
    void    disableFonts();
    void    addGroup();
    void    removeGroup();
    void    enableGroup();
    void    disableGroup();
    void    changeText();
    void    duplicateFonts();
    void    downloadFonts();
    void    print();
    void    printGroup();
    void    listingPercent(int p);
    void    refreshFontList();
    void    refreshFamilies();
    void    showInfo(const QString &info);
    void    setStatusBar();
    void    addFonts(const QSet<QUrl> &src);

    private:

    void    removeDeletedFontsFromGroups();
    void    selectGroup(CGroupListItem::EType grp);
    void    print(bool all);
    void    toggleGroup(bool enable);
    void    toggleFonts(bool enable, const QString &grp=QString());
    void    toggleFonts(CJobRunner::ItemList &urls, const QStringList &fonts, bool enable, const QString &grp);
    void    selectMainGroup();
    void    doCmd(CJobRunner::ECommand cmd, const CJobRunner::ItemList &urls, bool system=false);

    private:

    QSplitter        *itsGroupSplitter,
                     *itsPreviewSplitter;
    CFontPreview     *itsPreview;
    CPreviewListView *itsPreviewList;
    KConfig          itsConfig;
    QLabel           *itsStatusLabel;
    CProgressBar     *itsListingProgress;
    CFontList        *itsFontList;
    CFontListView    *itsFontListView;
    CGroupList       *itsGroupList;
    CGroupListView   *itsGroupListView;
    QPushButton      *itsDeleteGroupControl,
                     *itsEnableGroupControl,
                     *itsDisableGroupControl,
                     *itsAddFontControl,
                     *itsGetNewFontsControl,
                     *itsDeleteFontControl,
                     *itsScanDuplicateFontsControl;
    CFontFilter      *itsFilter;
    QString          itsLastStatusBarMsg;
    KIO::Job         *itsJob;
    QProgressDialog  *itsProgress;
    CUpdateDialog    *itsUpdateDialog;
    QTemporaryDir    *itsTempDir;
    QProcess         *itsPrintProc;
    QSet<QString>    itsDeletedFonts;
    QList<QUrl>       itsModifiedUrls;
    CJobRunner       *itsRunner;
    QMenu            *itsPreviewMenu,
                     *itsPreviewListMenu;
    QWidget          *itsPreviewWidget;
    bool             itsPreviewHidden;
};

}

#endif
