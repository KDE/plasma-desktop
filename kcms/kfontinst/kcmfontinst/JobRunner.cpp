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

#include "JobRunner.h"
#include "KfiConstants.h"
#include "Misc.h"
#include "Fc.h"
#include "ActionLabel.h"
#include <QTemporaryDir>
#include <KSharedConfig>
#include <KConfigGroup>
#include <kio/global.h>
#include <KIO/StatJob>
#include <KIO/FileCopyJob>
#include <KJobWidgets>
#include <QGridLayout>
#include <QProgressBar>
#include <QLabel>
#include <QX11Info>
#include <QStackedWidget>
#include <QCheckBox>
#include <QStyleOption>
#include <QStyle>
#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <QDBusServiceWatcher>
#include <fontconfig/fontconfig.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <X11/Xlib.h>
#include "config-fontinst.h"

#define CFG_GROUP                  "Runner Dialog"
#define CFG_DONT_SHOW_FINISHED_MSG "DontShowFinishedMsg"

namespace KFI
{

Q_GLOBAL_STATIC(FontInstInterface, theInterface)

FontInstInterface * CJobRunner::dbus()
{
    return theInterface;
}

QString CJobRunner::folderName(bool sys)
{
    if(!theInterface)
        return QString();

    QDBusPendingReply<QString> reply=theInterface->folderName(sys);

    reply.waitForFinished();
    return reply.isError() ? QString() : reply.argumentAt<0>();
}

void CJobRunner::startDbusService()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(OrgKdeFontinstInterface::staticInterfaceName())) {
        const QString fontinst = QStringLiteral(KFONTINST_LIB_EXEC_DIR"/fontinst");
        qDebug() << "Service " << OrgKdeFontinstInterface::staticInterfaceName() << " not registered, starting" << fontinst;
        QProcess::startDetached(fontinst, QStringList());
    }
}

static const int constDownloadFailed=-1;
static const int constInterfaceCheck=5*1000;

static void decode(const QUrl &url, Misc::TFont &font, bool &system)
{
    font=FC::decode(url);
    QUrlQuery query(url);
    system = (query.hasQueryItem("sys") && query.queryItemValue("sys") == QLatin1String("true"));
}

QUrl CJobRunner::encode(const QString &family, quint32 style, bool system)
{
    QUrl url(FC::encode(family, style));

    url.addQueryItem("sys", system ? "true" : "false");
    return url;
}

enum EPages
{
    PAGE_PROGRESS,
    PAGE_SKIP,
    PAGE_ERROR,
    PAGE_CANCEL,
    PAGE_COMPLETE
};

enum Response
{
    RESP_CONTINUE,
    RESP_AUTO,
    RESP_CANCEL
};

static void addIcon(QGridLayout *layout, QFrame *page, const char *iconName, int iconSize)
{
    QLabel *icon=new QLabel(page);
    icon->setPixmap(QIcon::fromTheme(iconName).pixmap(iconSize));
    icon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layout->addWidget(icon, 0, 0);
}

CJobRunner::CJobRunner(QWidget *parent, int xid)
           : QDialog(parent),
             itsIt(itsUrls.end()),
             itsEnd(itsIt),
             itsAutoSkip(false),
             itsCancelClicked(false),
             itsModified(false),
             itsTempDir(nullptr)
{
    setModal(true);

    if(nullptr==parent && 0!=xid)
        XSetTransientForHint(QX11Info::display(), winId(), xid);

    itsButtonBox = new QDialogButtonBox;
    connect(itsButtonBox, &QDialogButtonBox::clicked, this, &CJobRunner::slotButtonClicked);

    itsSkipButton = new QPushButton(i18n("Skip"));
    itsButtonBox->addButton(itsSkipButton, QDialogButtonBox::ActionRole);
    itsSkipButton->hide();
    itsAutoSkipButton = new QPushButton(i18n("AutoSkip"));
    itsButtonBox->addButton(itsAutoSkipButton, QDialogButtonBox::ActionRole);
    itsAutoSkipButton->hide();

    itsStack = new QStackedWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(itsStack);
    mainLayout->addWidget(itsButtonBox);

    QStyleOption option;
    option.initFrom(this);
    int iconSize=style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, this);
    
    QFrame *page = new QFrame(itsStack);
    QGridLayout *layout=new QGridLayout(page);
    itsStatusLabel=new QLabel(page);
    itsProgress=new QProgressBar(page);
//     itsStatusLabel->setWordWrap(true);
    layout->addWidget(itsActionLabel = new CActionLabel(this), 0, 0, 2, 1);
    layout->addWidget(itsStatusLabel, 0, 1);
    layout->addWidget(itsProgress, 1, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 2, 0);
    itsStack->insertWidget(PAGE_PROGRESS, page);

    page=new QFrame(itsStack);
    layout=new QGridLayout(page);
    itsSkipLabel=new QLabel(page);
    itsSkipLabel->setWordWrap(true);
    addIcon(layout, page, "dialog-error", iconSize);
    layout->addWidget(itsSkipLabel, 0, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);
    itsStack->insertWidget(PAGE_SKIP, page);

    page=new QFrame(itsStack);
    layout=new QGridLayout(page);
    itsErrorLabel=new QLabel(page);
    itsErrorLabel->setWordWrap(true);
    addIcon(layout, page, "dialog-error", iconSize);
    layout->addWidget(itsErrorLabel, 0, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);
    itsStack->insertWidget(PAGE_ERROR, page);

    page=new QFrame(itsStack);
    layout=new QGridLayout(page);
    QLabel *cancelLabel=new QLabel(i18n("<h3>Cancel?</h3><p>Are you sure you wish to cancel?</p>"), page);
    cancelLabel->setWordWrap(true);
    addIcon(layout, page, "dialog-warning", iconSize);
    layout->addWidget(cancelLabel, 0, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);
    itsStack->insertWidget(PAGE_CANCEL, page);

    if(KSharedConfig::openConfig(KFI_UI_CFG_FILE)->group(CFG_GROUP).readEntry(CFG_DONT_SHOW_FINISHED_MSG, false))
        itsDontShowFinishedMsg= nullptr;
    else
    {
        page=new QFrame(itsStack);
        layout=new QGridLayout(page);
        QLabel *finishedLabel=new QLabel(i18n("<h3>Finished</h3>"
                                            "<p>Please note that any open applications will need to be "
                                            "restarted in order for any changes to be noticed.</p>"),
                                        page);
        finishedLabel->setWordWrap(true);
        addIcon(layout, page, "dialog-information", iconSize);
        layout->addWidget(finishedLabel, 0, 1);
        layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);
        itsDontShowFinishedMsg = new QCheckBox(i18n("Do not show this message again"), page);
        itsDontShowFinishedMsg->setChecked(false);
        layout->addItem(new QSpacerItem(0, layout->spacing(), QSizePolicy::Fixed, QSizePolicy::Fixed), 2, 0);
        layout->addWidget(itsDontShowFinishedMsg, 3, 1);
        layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 4, 0);
        itsStack->insertWidget(PAGE_COMPLETE, page);
    }
    
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QLatin1String(OrgKdeFontinstInterface::staticInterfaceName()),
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange, this);

    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &CJobRunner::dbusServiceOwnerChanged);
    connect(dbus(), &OrgKdeFontinstInterface::status, this, &CJobRunner::dbusStatus);
    setMinimumSize(420, 160);
}

CJobRunner::~CJobRunner()
{
    delete itsTempDir;
}

void CJobRunner::getAssociatedUrls(const QUrl &url, QList<QUrl> &list, bool afmAndPfm, QWidget *widget)
{
    QString ext(url.path());
    int     dotPos(ext.lastIndexOf('.'));
    bool    check(false);

    if(-1==dotPos) // Hmm, no extension - check anyway...
        check=true;
    else           // Cool, got an extension - see if it is a Type1 font...
    {
        ext=ext.mid(dotPos+1);
        check=0==ext.compare("pfa", Qt::CaseInsensitive) ||
              0==ext.compare("pfb", Qt::CaseInsensitive);
    }

    if(check)
    {
        const char *afm[]={"afm", "AFM", "Afm", nullptr},
                   *pfm[]={"pfm", "PFM", "Pfm", nullptr};
        bool       gotAfm(false),
                   localFile(url.isLocalFile());
        int        e;

        for(e=0; afm[e]; ++e)
        {
            QUrl statUrl(url);
            statUrl.setPath(Misc::changeExt(url.path(), afm[e]));

            bool urlExists = false;
            if (localFile) {
                urlExists = Misc::fExists(statUrl.toLocalFile());
            } else {
                auto job = KIO::stat(statUrl);
                KJobWidgets::setWindow(job, widget);
                job->exec();
                urlExists = !job->error();
            }

            if (urlExists)
            {
                list.append(statUrl);
                gotAfm=true;
                break;
            }
        }

        if(afmAndPfm || !gotAfm)
            for(e=0; pfm[e]; ++e)
            {
                QUrl statUrl(url);
                statUrl.setPath(Misc::changeExt(url.path(), pfm[e]));

                bool urlExists = false;
                if (localFile) {
                    urlExists = Misc::fExists(statUrl.toLocalFile());
                } else {
                    auto job = KIO::stat(statUrl);
                    KJobWidgets::setWindow(job, widget);
                    job->exec();
                    urlExists = !job->error();
                }

                if (urlExists)
                {
                    list.append(statUrl);
                    break;
                }
            }
    }
}

static void addEnableActions(CJobRunner::ItemList &urls)
{
    CJobRunner::ItemList                modified;
    CJobRunner::ItemList::ConstIterator it(urls.constBegin()),
                                        end(urls.constEnd());
                            
    for(; it!=end; ++it)
    {
        if((*it).isDisabled)
        {
            CJobRunner::Item item(*it);
            item.fileName=QLatin1String("--");
            modified.append(item);
        }
        modified.append(*it);
    }
    
    urls=modified;
}

int CJobRunner::exec(ECommand cmd, const ItemList &urls, bool destIsSystem)
{
    itsAutoSkip=itsCancelClicked=itsModified=false;

    switch(cmd)
    {
        case CMD_INSTALL:
            setWindowTitle(i18n("Installing"));
            break;
        case CMD_DELETE:
            setWindowTitle(i18n("Uninstalling"));
            break;
        case CMD_ENABLE:
            setWindowTitle(i18n("Enabling"));
            break;
        case CMD_MOVE:
            setWindowTitle(i18n("Moving"));
            break;
        case CMD_UPDATE:
            setWindowTitle(i18n("Updating"));
            itsModified=true;
            break;
        case CMD_REMOVE_FILE:
            setWindowTitle(i18n("Removing"));
            break;
        default:
        case CMD_DISABLE:
            setWindowTitle(i18n("Disabling"));
    }

    itsDestIsSystem=destIsSystem;
    itsUrls=urls;
    if(CMD_INSTALL==cmd)
        std::sort(itsUrls.begin(), itsUrls.end());  // Sort list of fonts so that we have type1 fonts followed by their metrics...
    else if(CMD_MOVE==cmd)
        addEnableActions(itsUrls);
    itsIt=itsUrls.constBegin();
    itsEnd=itsUrls.constEnd();
    itsPrev=itsEnd;
    itsProgress->setValue(0);
    itsProgress->setRange(0, itsUrls.count()+1);
    itsProgress->show();
    itsCmd=cmd;
    itsCurrentFile=QString();
    itsStatusLabel->setText(QString());
    setPage(PAGE_PROGRESS);
    QTimer::singleShot(0, this, &CJobRunner::doNext);
    QTimer::singleShot(constInterfaceCheck, this, &CJobRunner::checkInterface);
    itsActionLabel->startAnimation();
    int rv=QDialog::exec();
    if(itsTempDir)
    {
        delete itsTempDir;
        itsTempDir= nullptr;
    }
    return rv;
}

void CJobRunner::doNext()
{
    if(itsIt==itsEnd/* || CMD_UPDATE==itsCmd*/)
    {
        if(itsModified)
        {
            // Force reconfig if command was already set to update...
            dbus()->reconfigure(getpid(), CMD_UPDATE==itsCmd);
            itsCmd=CMD_UPDATE;
            itsStatusLabel->setText(i18n("Updating font configuration. Please wait..."));
            itsProgress->setValue(itsProgress->maximum());
            emit configuring();
        }
        else
        {
            itsActionLabel->stopAnimation();
            if(PAGE_ERROR!=itsStack->currentIndex())
                reject();
        }
    }
    else
    {
        Misc::TFont font;
        bool        system;

        switch(itsCmd)
        {
            case CMD_INSTALL:
            {
                itsCurrentFile=fileName((*itsIt));

                if(itsCurrentFile.isEmpty()) // Failed to download...
                    dbusStatus(getpid(), constDownloadFailed);
                else
                {
                    // Create AFM if this is a PFM, and the previous was not the AFM for this font...
                    bool createAfm=Item::TYPE1_PFM==(*itsIt).type && 
                                   (itsPrev==itsEnd || (*itsIt).fileName!=(*itsPrev).fileName || Item::TYPE1_AFM!=(*itsPrev).type);
 
                    dbus()->install(itsCurrentFile, createAfm, itsDestIsSystem, getpid(), false);
                }
                break;
            }
            case CMD_DELETE:
                decode(*itsIt, font, system);
                dbus()->uninstall(font.family, font.styleInfo, system, getpid(), false);
                break;
            case CMD_ENABLE:
                decode(*itsIt, font, system);
                dbus()->enable(font.family, font.styleInfo, system, getpid(), false);
                break;
            case CMD_DISABLE:
                decode(*itsIt, font, system);
                dbus()->disable(font.family, font.styleInfo, system, getpid(), false);
                break;
            case CMD_MOVE:
                decode(*itsIt, font, system);
                // To 'Move' a disabled font, we first need to enable it. To accomplish this, JobRunner creates a 'fake' entry
                // with the filename "--"
                if((*itsIt).fileName==QLatin1String("--"))
                {
                    setWindowTitle(i18n("Enabling"));
                    dbus()->enable(font.family, font.styleInfo, system, getpid(), false);
                }
                else
                {
                    if(itsPrev!=itsEnd && (*itsPrev).fileName==QLatin1String("--"))
                        setWindowTitle(i18n("Moving"));
                    dbus()->move(font.family, font.styleInfo, itsDestIsSystem, getpid(), false);
                }
                break;
            case CMD_REMOVE_FILE:
                decode(*itsIt, font, system);
                dbus()->removeFile(font.family, font.styleInfo, (*itsIt).fileName, system, getpid(), false);
                break;
            default:
                break;
        }
        itsStatusLabel->setText(CMD_INSTALL==itsCmd ? (*itsIt).url() : FC::createName(FC::decode(*itsIt)));
        itsProgress->setValue(itsProgress->value()+1);
        
        // Keep copy of this iterator - so that can check whether AFM should be created.
        itsPrev=itsIt;
    }
}

void CJobRunner::checkInterface()
{
    if(itsIt==itsUrls.constBegin() && !FontInst::isStarted(dbus()))
    {
        setPage(PAGE_ERROR, i18n("Unable to start backend."));
        itsActionLabel->stopAnimation();
        itsIt=itsEnd;
    }
}

void CJobRunner::dbusServiceOwnerChanged(const QString &name, const QString &from, const QString &to)
{
    if(to.isEmpty() && !from.isEmpty() && name==QLatin1String(OrgKdeFontinstInterface::staticInterfaceName()) && itsIt!=itsEnd)
    {
        setPage(PAGE_ERROR, i18n("Backend died, but has been restarted. Please try again."));
        itsActionLabel->stopAnimation();
        itsIt=itsEnd;
    }
}

void CJobRunner::dbusStatus(int pid, int status)
{
    if(pid!=getpid())
        return;

    if(CMD_UPDATE==itsCmd)
    {
        setPage(PAGE_COMPLETE);
        return;
    }

    itsLastDBusStatus=status;

    if(itsCancelClicked)
    {
        itsActionLabel->stopAnimation();
        setPage(PAGE_CANCEL);
        return;
        /*
        if(RESP_CANCEL==itsResponse)
            itsIt=itsEnd;
        itsCancelClicked=false;
        setPage(PAGE_PROGRESS);
        itsActionLabel->startAnimation();
        */
    }

    // itsIt will equal itsEnd if user decided to cancel the current op
    if(itsIt==itsEnd)
    {
        doNext();
    }
    else if (0==status)
    {
        itsModified=true;
        ++itsIt;
        doNext();
    }
    else
    {
        bool    cont(itsAutoSkip && itsUrls.count()>1);
        QString currentName((*itsIt).fileName);

        if(!cont)
        {
            itsActionLabel->stopAnimation();

            if(FontInst::STATUS_SERVICE_DIED==status)
            {
                setPage(PAGE_ERROR, errorString(status));
                itsIt=itsEnd;
            }
            else
            {
                ItemList::ConstIterator lastPartOfCurrent(itsIt),
                                        next(itsIt==itsEnd ? itsEnd : itsIt+1);

                // If we're installing a Type1 font, and its already installed - then we need to skip past AFM/PFM
                if(next!=itsEnd && Item::TYPE1_FONT==(*itsIt).type &&
                   (*next).fileName==currentName && (Item::TYPE1_AFM==(*next).type || Item::TYPE1_PFM==(*next).type))
                {
                    next++;
                    if(next!=itsEnd && (*next).fileName==currentName && (Item::TYPE1_AFM==(*next).type || Item::TYPE1_PFM==(*next).type))
                        next++;
                }
                if(1==itsUrls.count() || next==itsEnd)
                    setPage(PAGE_ERROR, errorString(status));
                else
                {
                    setPage(PAGE_SKIP, errorString(status));
                    return;
                }
            }
        }

        contineuToNext(cont);
    }
}

void CJobRunner::contineuToNext(bool cont)
{
    itsActionLabel->startAnimation();
    if(cont)
    {
        if(CMD_INSTALL==itsCmd && Item::TYPE1_FONT==(*itsIt).type) // Did we error on a pfa/pfb? if so, exclude the afm/pfm...
        {
            QString currentName((*itsIt).fileName);

            ++itsIt;

            // Skip afm/pfm
            if(itsIt!=itsEnd && (*itsIt).fileName==currentName && (Item::TYPE1_AFM==(*itsIt).type || Item::TYPE1_PFM==(*itsIt).type))
                ++itsIt;
            // Skip pfm/afm
            if(itsIt!=itsEnd && (*itsIt).fileName==currentName && (Item::TYPE1_AFM==(*itsIt).type || Item::TYPE1_PFM==(*itsIt).type))
                ++itsIt;
        }
        else
            ++itsIt;
    }
    else
    {
        itsUrls.empty();
        itsIt=itsEnd=itsUrls.constEnd();
    }
    doNext();
}

void CJobRunner::slotButtonClicked(QAbstractButton *button)
{
    switch(itsStack->currentIndex())
    {
        case PAGE_PROGRESS:
            if(itsIt!=itsEnd)
                itsCancelClicked=true;
            break;
        case PAGE_SKIP:
            setPage(PAGE_PROGRESS);
            if (button == itsSkipButton) {
                contineuToNext(true);
            } else if (button == itsAutoSkipButton) {
                itsAutoSkip=true;
                contineuToNext(true);
            } else {
                contineuToNext(false);
            }

            break;
        case PAGE_CANCEL:
            if(button == itsButtonBox->button(QDialogButtonBox::Yes))
                itsIt=itsEnd;
            itsCancelClicked=false;
            setPage(PAGE_PROGRESS);
            itsActionLabel->startAnimation();
            // Now continue...
            dbusStatus(getpid(), itsLastDBusStatus);
            break;
        case PAGE_COMPLETE:
            if(itsDontShowFinishedMsg)
            {
                KConfigGroup grp(KSharedConfig::openConfig(KFI_UI_CFG_FILE)->group(CFG_GROUP));
                grp.writeEntry(CFG_DONT_SHOW_FINISHED_MSG, itsDontShowFinishedMsg->isChecked());
            }
        case PAGE_ERROR:
            QDialog::accept();
            break;
    }
}

void CJobRunner::closeEvent(QCloseEvent *e)
{
    if(PAGE_COMPLETE!=itsStack->currentIndex())
    {
        e->ignore();
        slotButtonClicked(PAGE_CANCEL==itsStack->currentIndex() ?
                          itsButtonBox->button(QDialogButtonBox::No) :
                          itsButtonBox->button(QDialogButtonBox::Cancel));
    }
}

void CJobRunner::setPage(int page, const QString &msg)
{
    itsStack->setCurrentIndex(page);

    switch(page)
    {
        case PAGE_PROGRESS:
            itsButtonBox->setStandardButtons(QDialogButtonBox::Cancel);
            itsSkipButton->hide();
            itsAutoSkipButton->hide();
            break;
        case PAGE_SKIP:
            itsSkipLabel->setText(i18n("<h3>Error</h3>")+QLatin1String("<p>")+msg+QLatin1String("</p>"));
            itsButtonBox->setStandardButtons(QDialogButtonBox::Cancel);
            itsSkipButton->show();
            itsAutoSkipButton->show();
            break;
        case PAGE_ERROR:
            itsErrorLabel->setText(i18n("<h3>Error</h3>")+QLatin1String("<p>")+msg+QLatin1String("</p>"));
            itsButtonBox->setStandardButtons(QDialogButtonBox::Cancel);
            itsSkipButton->hide();
            itsAutoSkipButton->hide();
            break;
        case PAGE_CANCEL:
            itsButtonBox->setStandardButtons(QDialogButtonBox::Yes|QDialogButtonBox::No);
            itsSkipButton->hide();
            itsAutoSkipButton->hide();
            break;
        case PAGE_COMPLETE:
            if(!itsDontShowFinishedMsg || itsDontShowFinishedMsg->isChecked()) {
                QDialog::accept();
            } else {
                itsButtonBox->setStandardButtons(QDialogButtonBox::Close);
                itsSkipButton->hide();
                itsAutoSkipButton->hide();
            }
            break;
    }
}

QString CJobRunner::fileName(const QUrl &url)
{
    if(url.isLocalFile())
        return url.toLocalFile();
    else
    {
        auto job = KIO::mostLocalUrl(url);
        job->exec();
        QUrl local = job->mostLocalUrl();

        if(local.isLocalFile())
            return local.toLocalFile(); // Yipee! no need to download!!
        else
        {
            // Need to do actual download...
            if(!itsTempDir)
            {
                itsTempDir=new QTemporaryDir(QDir::tempPath() + "/fontinst");
                itsTempDir->setAutoRemove(true);
            }

            QString tempName(itsTempDir->filePath(Misc::getFile(url.path())));
            auto job = KIO::file_copy(url, QUrl::fromLocalFile(tempName), -1, KIO::Overwrite);
            if (job->exec())
                return tempName;
            else
                return QString();
        }
    }
}

QString CJobRunner::errorString(int value) const
{
    Misc::TFont font(FC::decode(*itsIt));
    QString     urlStr;

    if(CMD_REMOVE_FILE==itsCmd)
        urlStr=(*itsIt).fileName;
    else if(font.family.isEmpty())
        urlStr=(*itsIt).url();
    else
        urlStr=FC::createName(font.family, font.styleInfo);
    
    switch(value)
    {
        case constDownloadFailed:
            return i18n("Failed to download <i>%1</i>", urlStr);
        case FontInst::STATUS_SERVICE_DIED:
            return i18n("System backend died. Please try again.<br><i>%1</i>", urlStr);
        case FontInst::STATUS_BITMAPS_DISABLED:
            return i18n("<i>%1</i> is a bitmap font, and these have been disabled on your system.", urlStr);
        case FontInst::STATUS_ALREADY_INSTALLED:
            return i18n("<i>%1</i> contains the font <b>%2</b>, which is already installed on your system.", urlStr,
                        FC::getName(itsCurrentFile));
        case FontInst::STATUS_NOT_FONT_FILE:
            return i18n("<i>%1</i> is not a font.", urlStr);
        case FontInst::STATUS_PARTIAL_DELETE:
            return i18n("Could not remove all files associated with <i>%1</i>", urlStr);
        case FontInst::STATUS_NO_SYS_CONNECTION:
            return i18n("Failed to start the system daemon.<br><i>%1</i>", urlStr);
        case KIO::ERR_FILE_ALREADY_EXIST:
        {
            QString name(Misc::modifyName(Misc::getFile((*itsIt).fileName))),
                    destFolder(Misc::getDestFolder(folderName(itsDestIsSystem), name));
            return i18n("<i>%1</i> already exists.", destFolder+name);
        }
        case KIO::ERR_DOES_NOT_EXIST:
            return i18n("<i>%1</i> does not exist.", urlStr);
        case KIO::ERR_WRITE_ACCESS_DENIED:
            return i18n("Permission denied.<br><i>%1</i>", urlStr);
        case KIO::ERR_UNSUPPORTED_ACTION:
            return i18n("Unsupported action.<br><i>%1</i>", urlStr);
        case KIO::ERR_CANNOT_AUTHENTICATE:
            return i18n("Authentication failed.<br><i>%1</i>", urlStr);
        default:
            return i18n("Unexpected error while processing: <i>%1</i>", urlStr);
    }
}

CJobRunner::Item::Item(const QUrl &u, const QString &n, bool dis)
                : QUrl(u), name(n), fileName(Misc::getFile(u.path())), isDisabled(dis)
{
    type=Misc::checkExt(fileName, "pfa") || Misc::checkExt(fileName, "pfb")
            ? TYPE1_FONT
            : Misc::checkExt(fileName, "afm")
                ? TYPE1_AFM
                : Misc::checkExt(fileName, "pfm")
                    ? TYPE1_PFM
                    : OTHER_FONT;

    if(OTHER_FONT!=type)
    {
        int pos(fileName.lastIndexOf('.'));

        if(-1!=pos)
            fileName.truncate(pos);
    }
}

CJobRunner::Item::Item(const QString &file, const QString &family, quint32 style, bool system)
                : QUrl(CJobRunner::encode(family, style, system)), fileName(file), type(OTHER_FONT)
{
}

bool CJobRunner::Item::operator<(const Item &o) const
{
    int nameComp(fileName.compare(o.fileName));

    return nameComp<0 || (0==nameComp && type<o.type);
}

}

