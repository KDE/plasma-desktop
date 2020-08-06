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

#include "DuplicatesDialog.h"
#include "ActionLabel.h"
#include "Misc.h"
#include "Fc.h"
#include "FcEngine.h"
#include "FontList.h"
#include "JobRunner.h"
#include <KIconLoader>
#include <KMessageBox>
#include <KFileItem>
#include <KPropertiesDialog>
#include <QLabel>
#include <KFormat>
#include <QMimeDatabase>
#include <QGridLayout>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QProcess>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace KFI
{

enum EDialogColumns
{
    COL_FILE,
    COL_TRASH,
    COL_SIZE,
    COL_DATE,
    COL_LINK
};

CDuplicatesDialog::CDuplicatesDialog(QWidget *parent, CFontList *fl)
                 : QDialog(parent),
                   itsFontList(fl)
{
    setWindowTitle(i18n("Duplicate Fonts"));
    itsButtonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    connect(itsButtonBox, &QDialogButtonBox::clicked, this, &CDuplicatesDialog::slotButtonClicked);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    setModal(true);

    QFrame *page = new QFrame(this);
    mainLayout->addWidget(page);
    mainLayout->addWidget(itsButtonBox);

    QGridLayout *layout=new QGridLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    itsLabel=new QLabel(page);
    itsView=new CFontFileListView(page);
    itsView->hide();
    layout->addWidget(itsActionLabel = new CActionLabel(this), 0, 0);
    layout->addWidget(itsLabel, 0, 1);
    itsLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    layout->addWidget(itsView, 1, 0, 1, 2);
    itsFontFileList=new CFontFileList(this);
    connect(itsFontFileList, SIGNAL(finished()), SLOT(scanFinished()));
    connect(itsView, &CFontFileListView::haveDeletions, this, &CDuplicatesDialog::enableButtonOk);
}

int CDuplicatesDialog::exec()
{
    itsActionLabel->startAnimation();
    itsLabel->setText(i18n("Scanning for duplicate fonts. Please wait..."));
    itsFontFileList->start();
    return QDialog::exec();
}

void CDuplicatesDialog::scanFinished()
{
    itsActionLabel->stopAnimation();

    if(itsFontFileList->wasTerminated())
    {
        itsFontFileList->wait();
        reject();
    }
    else
    {
        CFontFileList::TFontMap duplicates;

        itsFontFileList->getDuplicateFonts(duplicates);

        if(0==duplicates.count())
        {
            itsButtonBox->setStandardButtons(QDialogButtonBox::Close);
            itsLabel->setText(i18n("No duplicate fonts found."));
        }
        else
        {
            QSize sizeB4(size());

            itsButtonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Close);
            QPushButton *okButton = itsButtonBox->button(QDialogButtonBox::Ok);
            okButton->setDefault(true);
            okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
            okButton->setText(i18n("Delete Marked Files"));
            okButton->setEnabled(false);
            itsLabel->setText(i18np("%1 duplicate font found.", "%1 duplicate fonts found.", duplicates.count()));
            itsView->show();

            CFontFileList::TFontMap::ConstIterator it(duplicates.begin()),
                                                   end(duplicates.end());
            QFont                                  boldFont(font());

            boldFont.setBold(true);

            for(; it!=end; ++it)
            {
                QStringList details;

                details << FC::createName(it.key().family, it.key().styleInfo);

                CFontFileListView::StyleItem *top=new CFontFileListView::StyleItem(itsView, details,
                                                                                   it.key().family, it.key().styleInfo);

                QSet<QString>::ConstIterator fit((*it).begin()),
                                             fend((*it).end());
                int                          tt(0),
                                             t1(0);

                for(; fit!=fend; ++fit)
                {
                    QFileInfo info(*fit);
                    details.clear();
                    details.append(*fit);
                    details.append("");
                    details.append(KFormat().formatByteSize(info.size()));
                    details.append(QLocale().toString(info.created()));
                    if(info.isSymLink())
                        details.append(info.symLinkTarget());
                    new QTreeWidgetItem(top, details);
                    if(Misc::checkExt(*fit, "pfa") || Misc::checkExt(*fit, "pfb"))
                        t1++;
                    else
                        tt++;
                }
                top->setData(COL_FILE, Qt::DecorationRole,
                             QVariant(SmallIcon(t1>tt ? "application-x-font-type1" : "application-x-font-ttf")));
                top->setFont(COL_FILE, boldFont);
            }

            QTreeWidgetItem *item= nullptr;
            for(int i=0; (item=itsView->topLevelItem(i)); ++i)
                item->setExpanded(true);

            itsView->setSortingEnabled(true);
            itsView->header()->resizeSections(QHeaderView::ResizeToContents);

            int width=(itsView->frameWidth()+8)*2
                    + style()->pixelMetric(QStyle::PM_LayoutLeftMargin)
                    + style()->pixelMetric(QStyle::PM_LayoutRightMargin);

            for(int i=0; i<itsView->header()->count(); ++i)
                width+=itsView->header()->sectionSize(i);

            width=qMin(QApplication::desktop()->screenGeometry(this).width(), width);
            resize(width, height());
            QSize sizeNow(size());
            if(sizeNow.width()>sizeB4.width())
            {
                int xmod=(sizeNow.width()-sizeB4.width())/2,
                    ymod=(sizeNow.height()-sizeB4.height())/2;

                move(pos().x()-xmod, pos().y()-ymod);
            }
        }
    }
}

enum EStatus
{
    STATUS_NO_FILES,
    STATUS_ALL_REMOVED,
    STATUS_ERROR,
    STATUS_USER_CANCELLED
};

void CDuplicatesDialog::slotButtonClicked(QAbstractButton *button)
{
    switch(itsButtonBox->standardButton(button))
    {
        case QDialogButtonBox::Ok:
        {
            QSet<QString> files=itsView->getMarkedFiles();
            int           fCount=files.count();
            
            if(1==fCount
                ? KMessageBox::Yes==KMessageBox::warningYesNo(this,
                    i18n("Are you sure you wish to delete:\n%1", *files.begin()))
                : KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                    i18n("Are you sure you wish to delete:"), files.values()))
            {
                itsFontList->setSlowUpdates(true);
                    
                CJobRunner runner(this);

                connect(&runner, &CJobRunner::configuring, itsFontList, &CFontList::unsetSlowUpdates);
                runner.exec(CJobRunner::CMD_REMOVE_FILE, itsView->getMarkedItems(), false);
                itsFontList->setSlowUpdates(false);
                itsView->removeFiles();
                files=itsView->getMarkedFiles();
                if(fCount!=files.count())
                    CFcEngine::setDirty();
                if(0==files.count())
                    accept();
            }
            break;
        }
        case QDialogButtonBox::Cancel:
        case QDialogButtonBox::Close:
            if(!itsFontFileList->wasTerminated())
            {
                if(itsFontFileList->isRunning())
                {
                    if(KMessageBox::Yes==KMessageBox::warningYesNo(this, i18n("Cancel font scan?")))
                    {
                        itsLabel->setText(i18n("Canceling..."));

                        if(itsFontFileList->isRunning())
                            itsFontFileList->terminate();
                        else
                            reject();
                    }
                }
                else
                    reject();
            }
            break;
        default:
            break;
    }
}

void CDuplicatesDialog::enableButtonOk(bool on)
{
    QPushButton *okButton = itsButtonBox->button(QDialogButtonBox::Ok);
    if (okButton)
        okButton->setEnabled(on);
}

static uint qHash(const CFontFileList::TFile &key)
{
    return qHash(key.name.toLower());
}

CFontFileList::CFontFileList(CDuplicatesDialog *parent)
             : QThread(parent),
               itsTerminated(false)
{
}

void CFontFileList::start()
{
    if(!isRunning())
    {
        itsTerminated=false;
        QThread::start();
    }
}

void CFontFileList::terminate()
{
    itsTerminated=true;
}

void CFontFileList::getDuplicateFonts(TFontMap &map)
{
    map=itsMap;

    if(!map.isEmpty())
    {
        TFontMap::Iterator it(map.begin()),
                           end(map.end());

        // Now re-iterate, and remove any entries that only have 1 file...
        for(it=map.begin(); it!=end; )
            if((*it).count()<2)
                it=map.erase(it);
            else
                ++it;
    }
}

void CFontFileList::run()
{
    const QList<CFamilyItem *>          &families(((CDuplicatesDialog *)parent())->fontList()->families());
    QList<CFamilyItem *>::ConstIterator it(families.begin()),
                                        end(families.end());

    for(; it!=end; ++it)
    {
        QList<CFontItem *>::ConstIterator fontIt((*it)->fonts().begin()),
                                          fontEnd((*it)->fonts().end());

        for(; fontIt!=fontEnd; ++fontIt)
            if(!(*fontIt)->isBitmap())
            {
                Misc::TFont             font((*fontIt)->family(), (*fontIt)->styleInfo());
                FileCont::ConstIterator fileIt((*fontIt)->files().begin()),
                                        fileEnd((*fontIt)->files().end());

                for(; fileIt!=fileEnd; ++fileIt)
                    if(!Misc::isMetrics((*fileIt).path()) && !Misc::isBitmap((*fileIt).path()))
                        itsMap[font].insert((*fileIt).path());
            }
    }

    // if we have 2 fonts: /wibble/a.ttf and /wibble/a.TTF fontconfig only returns the 1st, so we
    // now iterate over fontconfig's list, and look for other matching fonts...
    if(!itsMap.isEmpty() && !itsTerminated)
    {
        // Create a map of folder -> set<files>
        TFontMap::Iterator           it(itsMap.begin()),
                                     end(itsMap.end());
        QHash<QString, QSet<TFile> > folderMap;

        for(int n=0; it!=end && !itsTerminated; ++it)
        {
            QStringList                   add;
            QSet<QString>::const_iterator fIt((*it).begin()),
                                          fEnd((*it).end());

            for(; fIt!=fEnd && !itsTerminated; ++fIt, ++n)
                folderMap[Misc::getDir(*fIt)].insert(TFile(Misc::getFile(*fIt), it));
        }

        // Go through our folder map, and check for file duplicates...
        QHash<QString, QSet<TFile> >::Iterator folderIt(folderMap.begin()),
                                               folderEnd(folderMap.end());

        for(; folderIt!=folderEnd && !itsTerminated; ++folderIt)
            fileDuplicates(folderIt.key(), *folderIt);
    }

    emit finished();
}

void CFontFileList::fileDuplicates(const QString &folder, const QSet<TFile> &files)
{
    QDir dir(folder);

    dir.setFilter(QDir::Files|QDir::Hidden);

    QFileInfoList list(dir.entryInfoList());

    for (int i = 0; i < list.size() && !itsTerminated; ++i)
    {
        QFileInfo fileInfo(list.at(i));

        // Check if this file is already know about - this will do a case-sensitive comparison
        if(!files.contains(TFile(fileInfo.fileName())))
        {
            // OK, not found - this means it is a duplicate, but different case. So, find the
            // FontMap iterator, and update its list of files.
            QSet<TFile>::ConstIterator entry=files.find(TFile(fileInfo.fileName(), true));

            if(entry!=files.end())
                (*((*entry).it)).insert(fileInfo.absoluteFilePath());
        }
    }
}

inline void markItem(QTreeWidgetItem *item)
{
    item->setData(COL_TRASH, Qt::DecorationRole, QVariant(SmallIcon("list-remove")));
}

inline void unmarkItem(QTreeWidgetItem *item)
{
    item->setData(COL_TRASH, Qt::DecorationRole, QVariant());
}

inline bool isMarked(QTreeWidgetItem *item)
{
    return item->data(COL_TRASH, Qt::DecorationRole).isValid();
}

CFontFileListView::CFontFileListView(QWidget *parent)
                 : QTreeWidget(parent)
{
    QStringList headers;
    headers.append(i18n("Font/File"));
    headers.append("");
    headers.append(i18n("Size"));
    headers.append(i18n("Date"));
    headers.append(i18n("Links To"));
    setHeaderLabels(headers);
    headerItem()->setData(COL_TRASH, Qt::DecorationRole, QVariant(SmallIcon("user-trash")));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setSelectionMode(ExtendedSelection);
    sortByColumn(COL_FILE, Qt::AscendingOrder);
    setSelectionBehavior(SelectRows);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);

    itsMenu=new QMenu(this);
    if(!Misc::app(KFI_VIEWER).isEmpty())
        itsMenu->addAction(QIcon::fromTheme("kfontview"), i18n("Open in Font Viewer"),
                           this, &CFontFileListView::openViewer);
    itsMenu->addAction(QIcon::fromTheme("document-properties"), i18n("Properties"),
                       this, &CFontFileListView::properties);
    itsMenu->addSeparator();
    itsUnMarkAct=itsMenu->addAction(i18n("Unmark for Deletion"),
                                    this, &CFontFileListView::unmark);
    itsMarkAct=itsMenu->addAction(QIcon::fromTheme("edit-delete"), i18n("Mark for Deletion"),
                                  this, &CFontFileListView::mark);

    connect(this, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(clicked(QTreeWidgetItem*,int)));
}

QSet<QString> CFontFileListView::getMarkedFiles()
{
    QTreeWidgetItem *root=invisibleRootItem();
    QSet<QString>   files;

    for(int t=0; t<root->childCount(); ++t)
    {
        QList<QTreeWidgetItem *> removeFiles;
        QTreeWidgetItem          *font=root->child(t);

        for(int c=0; c<font->childCount(); ++c)
        {
            QTreeWidgetItem *file=font->child(c);

            if(isMarked(file))
                files.insert(file->text(0));
        }
    }

    return files;
}

CJobRunner::ItemList CFontFileListView::getMarkedItems()
{
    QTreeWidgetItem      *root=invisibleRootItem();
    CJobRunner::ItemList items;
    QString              home(Misc::dirSyntax(QDir::homePath()));

    for(int t=0; t<root->childCount(); ++t)
    {
        QList<QTreeWidgetItem *> removeFiles;
        StyleItem                *style=(StyleItem *)root->child(t);

        for(int c=0; c<style->childCount(); ++c)
        {
            QTreeWidgetItem *file=style->child(c);

            if(isMarked(file))
                items.append(CJobRunner::Item(file->text(0), style->family(), style->value(), 0!=file->text(0).indexOf(home)));
        }
    }

    return items;
}

void CFontFileListView::removeFiles()
{
    QTreeWidgetItem          *root=invisibleRootItem();
    QList<QTreeWidgetItem *> removeFonts;

    for(int t=0; t<root->childCount(); ++t)
    {
        QList<QTreeWidgetItem *> removeFiles;
        QTreeWidgetItem          *font=root->child(t);

        for(int c=0; c<font->childCount(); ++c)
        {
            QTreeWidgetItem *file=font->child(c);

            if(!Misc::fExists(file->text(0)))
                removeFiles.append(file);
        }

        QList<QTreeWidgetItem *>::ConstIterator it(removeFiles.begin()),
                                                end(removeFiles.end());

        for(; it!=end; ++it)
            delete (*it);
        if(0==font->childCount())
            removeFonts.append(font);
    }

    QList<QTreeWidgetItem *>::ConstIterator it(removeFonts.begin()),
                                            end(removeFonts.end());
    for(; it!=end; ++it)
        delete (*it);
}

void CFontFileListView::openViewer()
{
    // Number of fonts user has selected, before we ask if they really want to view them all...
    static const int constMaxBeforePrompt=10;

    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;
    QSet<QString>            files;

    foreach(item, items)
        if(item->parent()) // Then it is a file, not font name :-)
            files.insert(item->text(0));

    if(!files.isEmpty() &&
       (files.count()<constMaxBeforePrompt ||
        KMessageBox::Yes==KMessageBox::questionYesNo(this, i18np("Open font in font viewer?",
                                                                 "Open all %1 fonts in font viewer?",
                                                                 files.count()))))
    {
         QSet<QString>::ConstIterator it(files.begin()),
                                      end(files.end());

        for(; it!=end; ++it)
        {
            QStringList args;

            args << (*it);

            QProcess::startDetached(Misc::app(KFI_VIEWER), args);
        }
    }
}

void CFontFileListView::properties()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;
    KFileItemList            files;
    QMimeDatabase db;

    foreach(item, items)
        if(item->parent())
            files.append(KFileItem(QUrl::fromLocalFile(item->text(0)),
                                   db.mimeTypeForFile(item->text(0)).name(),
                                   item->text(COL_LINK).isEmpty() ? S_IFREG : S_IFLNK));

    if(!files.isEmpty())
    {
        KPropertiesDialog dlg(files, this);
        dlg.exec();
    }
}

void CFontFileListView::mark()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;

    foreach(item, items)
        if(item->parent())
            markItem(item);
    checkFiles();
}

void CFontFileListView::unmark()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;

    foreach(item, items)
        if(item->parent())
            unmarkItem(item);
    checkFiles();
}

void CFontFileListView::selectionChanged()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;

    foreach(item, items)
        if(!item->parent() && item->isSelected())
            item->setSelected(false);
}

void CFontFileListView::clicked(QTreeWidgetItem *item, int col)
{
    if(item && COL_TRASH==col && item->parent())
    {
        if(isMarked(item))
            unmarkItem(item);
        else
            markItem(item);
        checkFiles();
    }
}

void CFontFileListView::contextMenuEvent(QContextMenuEvent *ev)
{
    QTreeWidgetItem *item(itemAt(ev->pos()));

    if(item && item->parent())
    {
        if(!item->isSelected())
            item->setSelected(true);

        bool haveUnmarked(false),
             haveMarked(false);

        QList<QTreeWidgetItem *> items(selectedItems());
        QTreeWidgetItem          *item;

        foreach(item, items)
        {
            if(item->parent() && item->isSelected())
            {
                if(isMarked(item))
                    haveMarked=true;
                else
                    haveUnmarked=true;
            }

            if(haveUnmarked && haveMarked)
                break;

        }

        itsMarkAct->setEnabled(haveUnmarked);
        itsUnMarkAct->setEnabled(haveMarked);
        itsMenu->popup(ev->globalPos());
    }
}

void CFontFileListView::checkFiles()
{
    // Need to check that if we mark a file that is linked to, then we also need
    // to mark the sym link.
    QSet<QString> marked(getMarkedFiles());

    if(marked.count())
    {
        QTreeWidgetItem *root=invisibleRootItem();

        for(int t=0; t<root->childCount(); ++t)
        {
            QTreeWidgetItem *font=root->child(t);

            for(int c=0; c<font->childCount(); ++c)
            {
                QTreeWidgetItem *file=font->child(c);
                QString         link(font->child(c)->text(COL_LINK));

                if(!link.isEmpty() && marked.contains(link))
                    if(!isMarked(file))
                        markItem(file);
            }
        }

        emit haveDeletions(true);
    }
    else
        emit haveDeletions(false);
}

}

