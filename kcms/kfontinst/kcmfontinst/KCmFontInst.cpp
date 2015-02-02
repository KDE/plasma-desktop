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

#include "KCmFontInst.h"
#include "KfiConstants.h"
#include "PrintDialog.h"
#include "FcEngine.h"
#include "FontPreview.h"
#include "FontsPackage.h"
#include "Misc.h"
#include "FontList.h"
#include "DuplicatesDialog.h"
#include "FontFilter.h"
#include "PreviewSelectAction.h"
#include "PreviewList.h"
#include <QGridLayout>
#include <QBoxLayout>
#include <QSplitter>
#include <QProgressBar>
#include <QApplication>
#include <QPainter>
#include <QMenu>
#include <QLabel>
#include <QCoreApplication>
#include <QProcess>
#include <QTextStream>
#include <KAboutData>
#include <KToolBar>
#include <KFileDialog>
#include <KMessageBox>
#include <KIO/Job>
#include <kio/netaccess.h>
#include <KPushButton>
#include <KGlobal>
#include <KGuiItem>
#include <KInputDialog>
#include <KIconLoader>
#include <KProgressDialog>
#include <KTemporaryFile>
#include <QIcon>
#include <KActionMenu>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KStandardAction>
#include <KZip>
#include <KNewStuff3/KNS3/DownloadDialog>
#include <KAction>
#include <QStandardPaths>

#define CFG_GROUP                  "Main Settings"
#define CFG_PREVIEW_SPLITTER_SIZES "PreviewSplitterSizes"
#define CFG_GROUP_SPLITTER_SIZES   "GroupSplitterSizes"
#define CFG_FONT_SIZE              "FontSize"

K_PLUGIN_FACTORY(FontInstallFactory,
        registerPlugin<KFI::CKCmFontInst>();
        )
K_EXPORT_PLUGIN(FontInstallFactory("fontinst"))

namespace KFI
{

static QString partialIcon(bool load=true)
{
    QString name = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/kfi/partial.png";

    if(Misc::fExists(name))
    {
        if(!load)
            QFile::remove(name);
    }
    else if(load)
    {
        QString pth;
        QPixmap pix=KIconLoader::global()->loadIcon("dialog-ok", KIconLoader::Small, KIconLoader::SizeSmall,
                                                    KIconLoader::DisabledState);

        pix.save(name, "PNG");
    }

    return name;
}

class CPushButton : public KPushButton
{
    public:

    CPushButton(const KGuiItem &item, QWidget *parent)
        : KPushButton(item, parent)
    {
        theirHeight=qMax(theirHeight, KPushButton::sizeHint().height());
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    QSize sizeHint() const
    {
        QSize sh(KPushButton::sizeHint());

        sh.setHeight(theirHeight);
        if(sh.width()<sh.height())
            sh.setWidth(sh.height());
        else if(text().isEmpty())
            sh.setWidth(theirHeight);
        return sh;
    }

    static int theirHeight;
};

class CToolBar : public KToolBar
{
    public:

    CToolBar(QWidget *parent)
        : KToolBar(parent)
    {
        setMovable(false);
        setFloatable(false);
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        setFont(QApplication::font());
    }

    void paintEvent(QPaintEvent *)
    {
        QColor col(palette().color(backgroundRole()));

        col.setAlphaF(0.0);
        QPainter(this).fillRect(rect(), col);
    }
};

class CProgressBar : public QProgressBar
{
    public:

    CProgressBar(QWidget *p, int h) : QProgressBar(p), itsHeight((int)(h*0.6))
        { setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); }

    virtual ~CProgressBar() { }

    int height() const     { return itsHeight; }
    QSize sizeHint() const { return QSize(100, itsHeight); }

    private:

    int itsHeight;
};

int CPushButton::theirHeight=0;

CKCmFontInst::CKCmFontInst(QWidget *parent, const QVariantList&)
            : KCModule(parent),
              itsPreview(NULL),
              itsConfig(KFI_UI_CFG_FILE),
              itsJob(NULL),
              itsProgress(NULL),
              itsUpdateDialog(NULL),
              itsTempDir(NULL),
              itsPrintProc(NULL),
              itsDownloadFontsAct(NULL)
{
    setButtons(Help);

    KIconLoader::global()->addAppDir(KFI_NAME);

    KAboutData *about = new KAboutData(QStringLiteral("fontinst"), i18n("KDE Font Manager"), QStringLiteral("1.0"), QString(),
                                       KAboutLicense::GPL, i18n("(C) Craig Drummond, 2000 - 2009"));
    about->addAuthor(i18n("Craig Drummond"), i18n("Developer and maintainer"), QStringLiteral("craig@kde.org"));
    setAboutData(about);

    KConfigGroup cg(&itsConfig, CFG_GROUP);

    itsGroupSplitter=new QSplitter(this);
    itsGroupSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QWidget *groupWidget=new QWidget(itsGroupSplitter),
            *fontWidget=new QWidget(itsGroupSplitter);

    itsPreviewSplitter=new QSplitter(fontWidget);
    itsPreviewSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QWidget     *toolbarWidget=new QWidget(this),
                *fontControlWidget=new QWidget(fontWidget);
    CToolBar    *toolbar=new CToolBar(toolbarWidget);
    QGridLayout *groupsLayout=new QGridLayout(groupWidget);
    QBoxLayout  *mainLayout=new QBoxLayout(QBoxLayout::TopToBottom, this),
                *toolbarLayout=new QBoxLayout(QBoxLayout::LeftToRight, toolbarWidget),
                *fontsLayout=new QBoxLayout(QBoxLayout::TopToBottom, fontWidget),
                *fontControlLayout=new QBoxLayout(QBoxLayout::LeftToRight, fontControlWidget);

    toolbarLayout->setMargin(0);
    toolbarLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    groupsLayout->setMargin(0);
    groupsLayout->setSpacing(KDialog::spacingHint());
    fontsLayout->setMargin(0);
    fontsLayout->setSpacing(KDialog::spacingHint());
    fontControlLayout->setMargin(0);
    fontControlLayout->setSpacing(KDialog::spacingHint());

    // Toolbar...
    KAction     *duplicateFontsAct=new KAction(QIcon::fromTheme("system-search"), i18n("Scan for Duplicate Fonts..."), this);
                //*validateFontsAct=new KAction(QIcon::fromTheme("checkmark"), i18n("Validate Fonts..."), this);

    if(!Misc::root())
        itsDownloadFontsAct=new KAction(QIcon::fromTheme("get-hot-new-stuff"), i18n("Get New Fonts..."), this);
    itsToolsMenu=new KActionMenu(QIcon::fromTheme("system-run"), i18n("Tools"), this);
    itsToolsMenu->addAction(duplicateFontsAct);
    //itsToolsMenu->addAction(validateFontsAct);
    if(itsDownloadFontsAct)
        itsToolsMenu->addAction(itsDownloadFontsAct);
    itsToolsMenu->setDelayed(false);
    toolbar->addAction(itsToolsMenu);
    itsFilter=new CFontFilter(toolbarWidget);

    // Details - Groups...
    itsGroupList=new CGroupList(groupWidget);
    itsGroupListView=new CGroupListView(groupWidget, itsGroupList);

    KPushButton *createGroup=new CPushButton(KGuiItem(QString(), "list-add",
                                                      i18n("Create a new group")),
                                             groupWidget);

    itsDeleteGroupControl=new CPushButton(KGuiItem(QString(), "edit-delete",
                                                   i18n("Remove group")),
                                          groupWidget);

    itsEnableGroupControl=new CPushButton(KGuiItem(QString(), "enablefont",
                                                   i18n("Enable all disabled fonts in the current group")),
                                          groupWidget);

    itsDisableGroupControl=new CPushButton(KGuiItem(QString(), "disablefont",
                                                    i18n("Disable all enabled fonts in the current group")),
                                           groupWidget);

    groupsLayout->addWidget(itsGroupListView, 0, 0, 1, 5);
    groupsLayout->addWidget(createGroup, 1, 0);
    groupsLayout->addWidget(itsDeleteGroupControl, 1, 1);
    groupsLayout->addWidget(itsEnableGroupControl, 1, 2);
    groupsLayout->addWidget(itsDisableGroupControl, 1, 3);
    groupsLayout->addItem(new QSpacerItem(itsDisableGroupControl->width(), KDialog::spacingHint(),
                          QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 4);

    itsPreviewWidget = new QWidget(this);
    QBoxLayout *previewWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, itsPreviewWidget);
    previewWidgetLayout->setMargin(0);
    previewWidgetLayout->setSpacing(0);
    
    // Preview
    QFrame     *previewFrame=new QFrame(itsPreviewWidget);
    QBoxLayout *previewFrameLayout=new QBoxLayout(QBoxLayout::LeftToRight, previewFrame);

    previewFrameLayout->setMargin(0);
    previewFrameLayout->setSpacing(0);
    previewFrame->setFrameShape(QFrame::StyledPanel);
    previewFrame->setFrameShadow(QFrame::Sunken);
    previewFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    itsPreview=new CFontPreview(previewFrame);
    itsPreview->setWhatsThis(i18n("This displays a preview of the selected font."));
    itsPreview->setContextMenuPolicy(Qt::CustomContextMenu);
    previewFrameLayout->addWidget(itsPreview);
    previewWidgetLayout->addWidget(previewFrame);
    itsPreview->engine()->readConfig(itsConfig);

    // List-style preview...
    itsPreviewList = new CPreviewListView(itsPreview->engine(), itsPreviewWidget);
    previewWidgetLayout->addWidget(itsPreviewList);
    itsPreviewList->setVisible(false);

    // Font List...
    itsFontList=new CFontList(fontWidget);
    itsFontListView=new CFontListView(itsPreviewSplitter, itsFontList);
    itsFontListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    itsAddFontControl=new CPushButton(KGuiItem(i18n("Add..."), "list-add",
                                               i18n("Install fonts")),
                                      fontControlWidget);

    itsDeleteFontControl=new CPushButton(KGuiItem(i18n("Delete"), "edit-delete",
                                                  i18n("Delete all selected fonts")),
                                         fontControlWidget);

    itsPreviewSplitter->addWidget(itsPreviewWidget);
    itsPreviewSplitter->setCollapsible(1, true);

    itsStatusLabel = new QLabel(fontControlWidget);
    itsStatusLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    itsListingProgress=new CProgressBar(fontControlWidget, itsStatusLabel->height());
    itsListingProgress->setRange(0, 100);

    // Layout widgets...
    toolbarLayout->addWidget(toolbar);
    toolbarLayout->addItem(new QSpacerItem(KDialog::spacingHint(), 2, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    toolbarLayout->addWidget(itsFilter);
    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(itsGroupSplitter);

    fontControlLayout->addWidget(itsAddFontControl);
    fontControlLayout->addWidget(itsDeleteFontControl);
    fontControlLayout->addWidget(itsStatusLabel);
    fontControlLayout->addItem(new QSpacerItem(0, itsListingProgress->height()+4,
                                               QSizePolicy::Fixed, QSizePolicy::Fixed));
    fontControlLayout->addWidget(itsListingProgress);

    fontsLayout->addWidget(itsPreviewSplitter);
    fontsLayout->addWidget(fontControlWidget);

    // Set size of widgets...
    itsPreviewSplitter->setChildrenCollapsible(false);
    itsGroupSplitter->setChildrenCollapsible(false);
    itsGroupSplitter->setStretchFactor(0, 0);
    itsGroupSplitter->setStretchFactor(1, 1);
    itsPreviewSplitter->setStretchFactor(0, 1);
    itsPreviewSplitter->setStretchFactor(1, 0);

    // Set sizes for 3 views...
    QList<int> defaultSizes;
    defaultSizes+=300;
    defaultSizes+=220;
    itsPreviewSplitter->setSizes(cg.readEntry(CFG_PREVIEW_SPLITTER_SIZES, defaultSizes));
    itsPreviewHidden=itsPreviewSplitter->sizes().at(1)<8;

    defaultSizes.clear();
    defaultSizes+=110;
    defaultSizes+=350;
    itsGroupSplitter->setSizes(cg.readEntry(CFG_GROUP_SPLITTER_SIZES, defaultSizes));

    // Preview widget pop-up menu
    itsPreviewMenu = new QMenu(itsPreview);
    QAction *zoomIn=KStandardAction::create(KStandardAction::ZoomIn, itsPreview, SLOT(zoomIn()), this),
            *zoomOut=KStandardAction::create(KStandardAction::ZoomOut, itsPreview, SLOT(zoomOut()), this);

    itsPreviewMenu->addAction(zoomIn);
    itsPreviewMenu->addAction(zoomOut);
    itsPreviewMenu->addSeparator();
    CPreviewSelectAction *prevSel=new CPreviewSelectAction(itsPreviewMenu);
    itsPreviewMenu->addAction(prevSel);
    KAction *changeTextAct=new KAction(QIcon::fromTheme("edit-rename"), i18n("Change Preview Text..."), this);
    itsPreviewMenu->addAction(changeTextAct),

    itsPreviewListMenu = new QMenu(itsPreviewList);
    itsPreviewListMenu->addAction(changeTextAct),

    // Connect signals...
    connect(itsPreview, SIGNAL(atMax(bool)), zoomIn, SLOT(setDisabled(bool)));
    connect(itsPreview, SIGNAL(atMin(bool)), zoomOut, SLOT(setDisabled(bool)));
    connect(prevSel, SIGNAL(range(QList<CFcEngine::TRange>)),
            itsPreview, SLOT(setUnicodeRange(QList<CFcEngine::TRange>)));
    connect(changeTextAct, SIGNAL(triggered(bool)), SLOT(changeText()));
    connect(itsFilter, SIGNAL(textChanged(QString)), itsFontListView, SLOT(filterText(QString)));
    connect(itsFilter, SIGNAL(criteriaChanged(int,qulonglong,QStringList)), 
            itsFontListView, SLOT(filterCriteria(int,qulonglong,QStringList)));
    connect(itsGroupListView, SIGNAL(del()), SLOT(removeGroup()));
    connect(itsGroupListView, SIGNAL(print()), SLOT(printGroup()));
    connect(itsGroupListView, SIGNAL(enable()), SLOT(enableGroup()));
    connect(itsGroupListView, SIGNAL(disable()), SLOT(disableGroup()));
    connect(itsGroupListView, SIGNAL(moveFonts()), SLOT(moveFonts()));
    connect(itsGroupListView, SIGNAL(zip()), SLOT(zipGroup()));
    connect(itsGroupListView, SIGNAL(itemSelected(QModelIndex)),
           SLOT(groupSelected(QModelIndex)));
    connect(itsGroupListView, SIGNAL(info(QString)),
           SLOT(showInfo(QString)));
    connect(itsGroupList, SIGNAL(refresh()), SLOT(refreshFontList()));
    connect(itsFontList, SIGNAL(listingPercent(int)), SLOT(listingPercent(int)));
    connect(itsFontList, SIGNAL(layoutChanged()), SLOT(setStatusBar()));
    connect(itsFontListView, SIGNAL(del()), SLOT(deleteFonts()));
    connect(itsFontListView, SIGNAL(print()), SLOT(print()));
    connect(itsFontListView, SIGNAL(enable()), SLOT(enableFonts()));
    connect(itsFontListView, SIGNAL(disable()), SLOT(disableFonts()));
    connect(itsFontListView, SIGNAL(fontsDropped(QSet<QUrl>)),
           SLOT(addFonts(QSet<QUrl>)));
    connect(itsFontListView, SIGNAL(itemsSelected(QModelIndexList)), SLOT(fontsSelected(QModelIndexList)));
    connect(itsFontListView, SIGNAL(refresh()), SLOT(setStatusBar()));
    connect(itsGroupListView, SIGNAL(unclassifiedChanged()), itsFontListView, SLOT(refreshFilter()));
    connect(createGroup, SIGNAL(clicked()), SLOT(addGroup()));
    connect(itsDeleteGroupControl, SIGNAL(clicked()), SLOT(removeGroup()));
    connect(itsEnableGroupControl, SIGNAL(clicked()), SLOT(enableGroup()));
    connect(itsDisableGroupControl, SIGNAL(clicked()), SLOT(disableGroup()));
    connect(itsAddFontControl, SIGNAL(clicked()), SLOT(addFonts()));
    connect(itsDeleteFontControl, SIGNAL(clicked()), SLOT(deleteFonts()));
    connect(duplicateFontsAct, SIGNAL(triggered(bool)), SLOT(duplicateFonts()));
    //connect(validateFontsAct, SIGNAL(triggered(bool)), SLOT(validateFonts()));
    if(itsDownloadFontsAct)
        connect(itsDownloadFontsAct, SIGNAL(triggered(bool)), SLOT(downloadFonts()));
    connect(itsPreview, SIGNAL(customContextMenuRequested(QPoint)), SLOT(previewMenu(QPoint)));
    connect(itsPreviewList, SIGNAL(showMenu(QPoint)), SLOT(previewMenu(QPoint)));
    connect(itsPreviewSplitter, SIGNAL(splitterMoved(int,int)), SLOT(splitterMoved()));

    selectMainGroup();
    itsFontList->load();
}

CKCmFontInst::~CKCmFontInst()
{
    KConfigGroup cg(&itsConfig, CFG_GROUP);

    cg.writeEntry(CFG_PREVIEW_SPLITTER_SIZES, itsPreviewSplitter->sizes());
    cg.writeEntry(CFG_GROUP_SPLITTER_SIZES, itsGroupSplitter->sizes());
    delete itsTempDir;
    partialIcon(false);
}

QString CKCmFontInst::quickHelp() const
{
    return Misc::root()
               ? i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy one into the folder.</p>")
               : i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy it into the appropriate folder - "
                      " \"%1\" for fonts available to just yourself, or "
                      " \"%2\" for system-wide fonts (available to all).</p>",
                      i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS));
}

void CKCmFontInst::previewMenu(const QPoint &pos)
{
    if(itsPreviewList->isHidden())
        itsPreviewMenu->popup(itsPreview->mapToGlobal(pos));
    else
        itsPreviewListMenu->popup(itsPreviewList->mapToGlobal(pos));
}

void CKCmFontInst::splitterMoved()
{
    if(itsPreviewWidget->width()>8 && itsPreviewHidden)
    {
        itsPreviewHidden=false;
        fontsSelected(itsFontListView->getSelectedItems());
    }
    else if(!itsPreviewHidden && itsPreviewWidget->width()<8)
        itsPreviewHidden=true;
}

void CKCmFontInst::fontsSelected(const QModelIndexList &list)
{
    if(!itsPreviewHidden)
    {
        if(list.count())
        {
            if(list.count()<2)
            {
                CFontModelItem *mi=static_cast<CFontModelItem *>(list.last().internalPointer());
                CFontItem      *font=mi->parent()
                                        ? static_cast<CFontItem *>(mi)
                                        : (static_cast<CFamilyItem *>(mi))->regularFont();

                if(font)
                    itsPreview->showFont(font->isEnabled() ? font->family() : font->fileName(),
                                         font->styleInfo(), font->index());
            }
            else
                itsPreviewList->showFonts(list);
        }
        itsPreviewList->setVisible(list.count()>1);
        itsPreview->parentWidget()->setVisible(list.count()<2);
    }

    itsDeleteFontControl->setEnabled(list.count());
}

void CKCmFontInst::addFonts()
{
    QList<QUrl> list=KFileDialog::getOpenUrls(QUrl(), CFontList::fontMimeTypes.join(" "), this, i18n("Add Fonts"));

    if(list.count())
    {
        QSet<QUrl>           urls;
        QList<QUrl>::Iterator it(list.begin()),
                             end(list.end());

        for(; it!=end; ++it)
        {
            if(KFI_KIO_FONTS_PROTOCOL!=(*it).scheme()) // Do not try to install from fonts:/ !!!
            {
                QUrl url(KIO::NetAccess::mostLocalUrl(*it, this));

                if(url.isLocalFile())
                {
                    QString file(url.toLocalFile());

                    if(Misc::isPackage(file)) // If its a package we need to unzip 1st...
                        urls+=FontsPackage::extract(url.toLocalFile(), &itsTempDir);
                    else if(!Misc::isMetrics(url))
                        urls.insert(url);
                }
                else if(!Misc::isMetrics(url))
                    urls.insert(url);
            }
        }
        if(urls.count())
            addFonts(urls);
        delete itsTempDir;
        itsTempDir=NULL;
    }
}

void CKCmFontInst::groupSelected(const QModelIndex &index)
{
    CGroupListItem *grp=NULL;

    if(index.isValid())
        grp=static_cast<CGroupListItem *>(index.internalPointer());

    itsFontListView->setFilterGroup(grp);
    setStatusBar();

    //
    // Check fonts listed within group are still valid!
    if(grp && grp->isCustom() && !grp->validated())
    {
        QSet<QString>           remList;
        QSet<QString>::Iterator it(grp->families().begin()),
                                end(grp->families().end());

        for(; it!=end; ++it)
            if(!itsFontList->hasFamily(*it))
                remList.insert(*it);
        it=remList.begin();
        end=remList.end();
        for(; it!=end; ++it)
            itsGroupList->removeFromGroup(grp, *it);
        grp->setValidated();
    }

    if(itsDownloadFontsAct)
        itsDownloadFontsAct->setEnabled(grp->isPersonal() || grp->isAll());
}

void CKCmFontInst::print(bool all)
{
    //
    // In order to support printing of newly installed/enabled fonts, the actual printing
    // is carried out by the kfontinst helper app. This way we know Qt's font list will be
    // up to date.
    if((!itsPrintProc || QProcess::NotRunning==itsPrintProc->state()) && !Misc::app(KFI_PRINTER).isEmpty())
    {
        QSet<Misc::TFont> fonts;

        itsFontListView->getPrintableFonts(fonts, !all);

        if(fonts.count())
        {
            CPrintDialog dlg(this);
            KConfigGroup cg(&itsConfig, CFG_GROUP);

            if(dlg.exec(cg.readEntry(CFG_FONT_SIZE, 1)))
            {
                static const int constSizes[]={0, 12, 18, 24, 36, 48};
                QSet<Misc::TFont>::ConstIterator it(fonts.begin()),
                                                 end(fonts.end());
                KTemporaryFile                   tmpFile;
                bool                             useFile(fonts.count()>16),
                                                 startProc(true);
                QStringList                      args;

                if(!itsPrintProc)
                    itsPrintProc=new QProcess(this);
                else
                    itsPrintProc->kill();

                //
                // If we have lots of fonts to print, pass kfontinst a tempory groups file to print
                // instead of passing font by font...
                if(useFile)
                {
                    if(tmpFile.open())
                    {
                        QTextStream str(&tmpFile);

                        for(; it!=end; ++it)
                            str << (*it).family << endl
                                << (*it).styleInfo << endl;

                        args << "--embed" << QString().sprintf("0x%x", (unsigned int)window()->winId())
                             << "--caption" << KGlobal::caption().toUtf8()
                             << "--icon" << "preferences-desktop-font-installer"
                             << "--size" << QString().setNum(constSizes[dlg.chosenSize() < 6 ? dlg.chosenSize() : 2])
                             << "--listfile" << tmpFile.fileName()
                             << "--deletefile";
                    }
                    else
                    {
                        KMessageBox::error(this, i18n("Failed to save list of fonts to print."));
                        startProc=false;
                    }
                }
                else
                {
                    args << "--embed" << QString().sprintf("0x%x", (unsigned int)window()->winId())
                         << "--caption" << KGlobal::caption().toUtf8()
                         << "--icon" << "preferences-desktop-font-installer"
                         << "--size" << QString().setNum(constSizes[dlg.chosenSize()<6 ? dlg.chosenSize() : 2]);

                    for(; it!=end; ++it)
                        args << "--pfont" << QString((*it).family.toUtf8()+','+QString().setNum((*it).styleInfo));
                }

                if(startProc)
                {
                    itsPrintProc->start(Misc::app(KFI_PRINTER), args);

                    if(itsPrintProc->waitForStarted(1000))
                    {
                        if(useFile)
                            tmpFile.setAutoRemove(false);
                    }
                    else
                        KMessageBox::error(this, i18n("Failed to start font printer."));
                }
                cg.writeEntry(CFG_FONT_SIZE, dlg.chosenSize());
            }
        }
        else
            KMessageBox::information(this, i18n("There are no printable fonts.\n"
                                                "You can only print non-bitmap and enabled fonts."),
                                           i18n("Cannot Print"));
    }
}

void CKCmFontInst::deleteFonts()
{
    CJobRunner::ItemList urls;
    QStringList          fontNames;
    QSet<Misc::TFont>    fonts;

    itsDeletedFonts.clear();
    itsFontListView->getFonts(urls, fontNames, &fonts, true);

    if(urls.isEmpty())
        KMessageBox::information(this, i18n("You did not select anything to delete."),
                                       i18n("Nothing to Delete"));
    else
    {
        QSet<Misc::TFont>::ConstIterator it(fonts.begin()),
                                         end(fonts.end());
        bool                             doIt=false;

        for(; it!=end; ++it)
            itsDeletedFonts.insert((*it).family);

        switch(fontNames.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                        i18n("<p>Do you really want to "
                                "delete</p><p>\'<b>%1</b>\'?</p>", fontNames.first()),
                        i18n("Delete Font"), KStandardGuiItem::del());
            break;
            default:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                        i18np("Do you really want to delete this font?",
                                "Do you really want to delete these %1 fonts?",
                                fontNames.count()),
                        fontNames, i18n("Delete Fonts"), KStandardGuiItem::del());
        }

        if(doIt)
        {
            itsStatusLabel->setText(i18n("Deleting font(s)..."));
            doCmd(CJobRunner::CMD_DELETE, urls);
        }
    }
}

void CKCmFontInst::moveFonts()
{
    CJobRunner::ItemList urls;
    QStringList          fontNames;

    itsDeletedFonts.clear();
    itsFontListView->getFonts(urls, fontNames, NULL, true);

    if(urls.isEmpty())
        KMessageBox::information(this, i18n("You did not select anything to move."),
                                       i18n("Nothing to Move"));
    else
    {
        bool doIt=false;

        switch(fontNames.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                        i18n("<p>Do you really want to "
                             "move</p><p>\'<b>%1</b>\'</p><p>from <i>%2</i> to <i>%3</i>?</p>",
                             fontNames.first(),
                             itsGroupListView->isSystem() ? i18n(KFI_KIO_FONTS_SYS) : i18n(KFI_KIO_FONTS_USER),
                             itsGroupListView->isSystem() ? i18n(KFI_KIO_FONTS_USER) : i18n(KFI_KIO_FONTS_SYS)),
                        i18n("Move Font"), KGuiItem(i18n("Move")));
            break;
            default:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                        i18np("<p>Do you really want to move this font from <i>%2</i> to <i>%3</i>?</p>",
                              "<p>Do you really want to move these %1 fonts from <i>%2</i> to <i>%3</i>?</p>",
                              fontNames.count(),
                              itsGroupListView->isSystem() ? i18n(KFI_KIO_FONTS_SYS) : i18n(KFI_KIO_FONTS_USER),
                              itsGroupListView->isSystem() ? i18n(KFI_KIO_FONTS_USER) : i18n(KFI_KIO_FONTS_SYS)),
                        fontNames, i18n("Move Fonts"), KGuiItem(i18n("Move")));
        }

        if(doIt)
        {
            itsStatusLabel->setText(i18n("Moving font(s)..."));
            doCmd(CJobRunner::CMD_MOVE, urls, !itsGroupListView->isSystem());
        }
    }
}

void CKCmFontInst::zipGroup()
{
    QModelIndex idx(itsGroupListView->currentIndex());

    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp)
        {
            QString fileName=KFileDialog::getSaveFileName(QUrl::fromLocalFile(grp->name()), QStringLiteral("application/zip"), this, i18n("Export Group"),
                                                          KFileDialog::ConfirmOverwrite);

            if(!fileName.isEmpty())
            {
                KZip zip(fileName);

                if(zip.open(QIODevice::WriteOnly))
                {
                    QSet<QString> files;

                    files=itsFontListView->getFiles();

                    if(files.count())
                    {
                        QMap<QString, QString>                map=Misc::getFontFileMap(files);
                        QMap<QString, QString>::ConstIterator it(map.constBegin()),
                                                            end(map.constEnd());

                        for(; it!=end; ++it)
                            zip.addLocalFile(it.value(), it.key());
                        zip.close();
                    }
                    else
                        KMessageBox::error(this, i18n("No files?"));
                }
                else
                    KMessageBox::error(this, i18n("Failed to open %1 for writing", fileName));
            }
        }
    }
}

void CKCmFontInst::enableFonts()
{
    toggleFonts(true);
}

void CKCmFontInst::disableFonts()
{
    toggleFonts(false);
}

void CKCmFontInst::addGroup()
{
    bool    ok;
    QString name(KInputDialog::getText(i18n("Create New Group"),
                                       i18n("Please enter the name of the new group:"),
                                       i18n("New Group"), &ok, this));

    if(ok && !name.isEmpty())
        itsGroupList->createGroup(name);
}

void CKCmFontInst::removeGroup()
{
    if(itsGroupList->removeGroup(itsGroupListView->currentIndex()))
        selectMainGroup();
}

void CKCmFontInst::enableGroup()
{
    toggleGroup(true);
}

void CKCmFontInst::disableGroup()
{
    toggleGroup(false);
}

void CKCmFontInst::changeText()
{
    bool             status;
    QRegExpValidator validator(QRegExp(".*"), 0L);
    QString          oldStr(itsPreview->engine()->getPreviewString()),
                     newStr(KInputDialog::getText(i18n("Preview Text"),
                                                  i18n("Please enter new text:"),
                                                  oldStr, &status, this, &validator));

    if(status && oldStr!=newStr)
    {
        itsPreview->engine()->setPreviewString(newStr);

        itsPreview->showFont();
        itsPreviewList->refreshPreviews();
    }
}

void CKCmFontInst::duplicateFonts()
{
    CDuplicatesDialog(this, itsFontList).exec();
}

//void CKCmFontInst::validateFonts()
//{
//}

void CKCmFontInst::downloadFonts()
{
    KNS3::DownloadDialog *newStuff = new KNS3::DownloadDialog("kfontinst.knsrc", this);
    newStuff->exec();

    if(newStuff->changedEntries().count())  // We have new fonts, so need to reconfigure fontconfig...
    {
        // Ask dbus helper for the current fonts folder name...
        // We then sym-link our knewstuff3 download folder into the fonts folder...
        QString destFolder=CJobRunner::folderName(false);
        if(!destFolder.isEmpty()) {
            destFolder+="kfontinst";
            if(!QFile::exists(destFolder)) {
                QFile _file(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + "kfontinst");
                _file.link(destFolder);
            }
        }

        doCmd(CJobRunner::CMD_UPDATE, CJobRunner::ItemList());
    }

    delete newStuff;
}

void CKCmFontInst::print()
{
    print(false);
}

void CKCmFontInst::printGroup()
{
    print(true);
}

void CKCmFontInst::listingPercent(int p)
{
    if(0==p)
    {
        showInfo(i18n("Scanning font list..."));
        itsListingProgress->show();
    }
    else if(100==p && p!=itsListingProgress->value())
    {
        removeDeletedFontsFromGroups();

        QSet<QString> foundries;

        itsFontList->getFoundries(foundries);
        itsFilter->setFoundries(foundries);
        refreshFamilies();
        itsListingProgress->hide();
        itsFontListView->selectFirstFont();
    }
    itsListingProgress->setValue(p);
}

void CKCmFontInst::refreshFontList()
{
    itsFontListView->refreshFilter();
    refreshFamilies();
}

void CKCmFontInst::refreshFamilies()
{
    QSet<QString> enabledFamilies,
                  disabledFamilies,
                  partialFamilies;

    itsFontList->getFamilyStats(enabledFamilies, disabledFamilies, partialFamilies);
    itsGroupList->updateStatus(enabledFamilies, disabledFamilies, partialFamilies);
    setStatusBar();
}

void CKCmFontInst::showInfo(const QString &info)
{
    if(info.isEmpty())
        if(itsLastStatusBarMsg.isEmpty())
            setStatusBar();
        else
        {
            itsStatusLabel->setText(itsLastStatusBarMsg);
            itsLastStatusBarMsg=QString();
        }
    else
    {
        if(itsLastStatusBarMsg.isEmpty())
            itsLastStatusBarMsg=itsStatusLabel->text();
        itsStatusLabel->setText(info);
    }
}

void CKCmFontInst::setStatusBar()
{
    if(itsFontList->slowUpdates())
        return;

    int  enabled=0,
         disabled=0,
         partial=0;
    bool selectedEnabled=false,
         selectedDisabled=false;

    itsStatusLabel->setToolTip(QString());
    if(0==itsFontList->families().count())
        itsStatusLabel->setText(i18n("No fonts"));
    else
    {
        itsFontListView->stats(enabled, disabled, partial);
        itsFontListView->selectedStatus(selectedEnabled, selectedDisabled);

        QString text(i18np("1 Font", "%1 Fonts", enabled+disabled+partial));

        if(disabled||partial)
        {
            text+=QString(" (<img src=\"%1\" />%2").arg(KIconLoader::global()->iconPath("dialog-ok", -KIconLoader::SizeSmall)).arg(enabled)
                 +QString(" <img src=\"%1\" />%2").arg(KIconLoader::global()->iconPath("dialog-cancel", -KIconLoader::SizeSmall)).arg(disabled);
            if(partial)
                text+=QString(" <img src=\"%1\" />%2").arg(partialIcon()).arg(partial);
            text+=QLatin1Char(')');
            itsStatusLabel->setToolTip(partial
                                        ? i18n("<table>"
                                               "<tr><td align=\"right\">Enabled:</td><td>%1</td></tr>"
                                               "<tr><td align=\"right\">Disabled:</td><td>%2</td></tr>"
                                               "<tr><td align=\"right\">Partially enabled:</td><td>%3</td></tr>"
                                               "<tr><td align=\"right\">Total:</td><td>%4</td></tr>"
                                               "</table>", enabled, disabled, partial, enabled+disabled+partial)
                                        : i18n("<table>"
                                               "<tr><td align=\"right\">Enabled:</td><td>%1</td></tr>"
                                               "<tr><td align=\"right\">Disabled:</td><td>%2</td></tr>"
                                               "<tr><td align=\"right\">Total:</td><td>%3</td></tr>"
                                               "</table>", enabled, disabled, enabled+disabled));
        }

        itsStatusLabel->setText(disabled||partial ? "<p>"+text+"</p>" : text);
    }

    CGroupListItem::EType type(itsGroupListView->getType());

    bool isStd(CGroupListItem::CUSTOM==type);

    itsAddFontControl->setEnabled(CGroupListItem::ALL==type||CGroupListItem::UNCLASSIFIED==type||
                                  CGroupListItem::PERSONAL==type||CGroupListItem::SYSTEM==type);
    itsDeleteGroupControl->setEnabled(isStd);
    itsEnableGroupControl->setEnabled(disabled||partial);
    itsDisableGroupControl->setEnabled(isStd && (enabled||partial));

    itsGroupListView->controlMenu(itsDeleteGroupControl->isEnabled(),
                                  itsEnableGroupControl->isEnabled(),
                                  itsDisableGroupControl->isEnabled(), enabled||partial, enabled||disabled);

    itsDeleteFontControl->setEnabled(selectedEnabled||selectedDisabled);
}

void CKCmFontInst::addFonts(const QSet<QUrl> &src)
{
    if(src.count() && !itsGroupListView->isCustom())
    {
        bool system;

        if(Misc::root())
            system=true;
        else
        {
            switch(itsGroupListView->getType())
            {
                case CGroupListItem::ALL:
                case CGroupListItem::UNCLASSIFIED:
                    switch(KMessageBox::questionYesNoCancel(this,
                           i18n("Do you wish to install the font(s) for personal use "
                                "(only available to you), or "
                                "system-wide (available to all users)?"),
                           i18n("Where to Install"), KGuiItem(i18n(KFI_KIO_FONTS_USER)),
                                KGuiItem(i18n(KFI_KIO_FONTS_SYS))))
                    {
                        case KMessageBox::Yes:
                            system=false;
                            break;
                        case KMessageBox::No:
                            system=true;
                            break;
                        default:
                        case KMessageBox::Cancel:
                            return;
                    }
                    break;
                case CGroupListItem::PERSONAL:
                    system=false;
                    break;
                case CGroupListItem::SYSTEM:
                    system=true;
                    break;
                default:
                    return;
            }
        }

        QSet<QUrl>                copy;
        QSet<QUrl>::ConstIterator it,
                                  end(src.end());

        //
        // Check if font has any associated AFM or PFM file...
        itsStatusLabel->setText(i18n("Looking for any associated files..."));

        if(!itsProgress)
        {
            itsProgress=new KProgressDialog(this, i18n("Scanning Files..."),
                                            i18n("Looking for additional files to install..."));
            itsProgress->setModal(true);
            itsProgress->setAutoReset(true);
            itsProgress->setAutoClose(true);
        }

        itsProgress->setAllowCancel(false);
        itsProgress->setMinimumDuration(500);
        itsProgress->progressBar()->show();
        itsProgress->progressBar()->setRange(0, src.size());
        itsProgress->progressBar()->setValue(0);

        int steps=src.count()<200 ? 1 : src.count()/10;
        for(it=src.begin(); it!=end; ++it)
        {
            QList<QUrl> associatedUrls;

            itsProgress->setLabelText(i18n("Looking for files associated with %1", (*it).url()));
            itsProgress->progressBar()->setValue(itsProgress->progressBar()->value()+1);
            if(1==steps || 0==(itsProgress->progressBar()->value()%steps))
            {
                bool dialogVisible(itsProgress->isVisible());
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                if(dialogVisible && !itsProgress->isVisible()) // User closed dialog! re-open!!!
                    itsProgress->show();
            }

            CJobRunner::getAssociatedUrls(*it, associatedUrls, false, this);
            copy.insert(*it);

            QList<QUrl>::Iterator aIt(associatedUrls.begin()),
                                 aEnd(associatedUrls.end());

            for(; aIt!=aEnd; ++aIt)
                copy.insert(*aIt);
        }
        itsProgress->close();

        CJobRunner::ItemList installUrls;

        end=copy.end();
        for(it=copy.begin(); it!=end; ++it)
            installUrls.append(*it);

        itsStatusLabel->setText(i18n("Installing font(s)..."));
        doCmd(CJobRunner::CMD_INSTALL, installUrls, system);
    }
}

void CKCmFontInst::removeDeletedFontsFromGroups()
{
    if(itsDeletedFonts.count())
    {
        QSet<QString>::Iterator it(itsDeletedFonts.begin()),
                                end(itsDeletedFonts.end());

        for(; it!=end; ++it)
            if(!itsFontList->hasFamily(*it))
                itsGroupList->removeFamily(*it);

        itsDeletedFonts.clear();
    }
}
        
void CKCmFontInst::selectGroup(CGroupListItem::EType grp)
{
    QModelIndex current(itsGroupListView->currentIndex());

    if(current.isValid())
    {
        CGroupListItem *grpItem=static_cast<CGroupListItem *>(current.internalPointer());

        if(grpItem && grp==grpItem->type())
            return;
        else
            itsGroupListView->selectionModel()->select(current,
                                                       QItemSelectionModel::Deselect);
    }

    QModelIndex idx(itsGroupList->index(grp));

    itsGroupListView->selectionModel()->select(idx, QItemSelectionModel::Select);
    itsGroupListView->setCurrentIndex(idx);
    groupSelected(idx);
    itsFontListView->refreshFilter();
    setStatusBar();
}

void CKCmFontInst::toggleGroup(bool enable)
{
    QModelIndex idx(itsGroupListView->currentIndex());

    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp)
            toggleFonts(enable, grp->name());
    }
}

void CKCmFontInst::toggleFonts(bool enable, const QString &grp)
{
    CJobRunner::ItemList urls;
    QStringList          fonts;

    itsFontListView->getFonts(urls, fonts, NULL, grp.isEmpty(), !enable, enable);

    if(urls.isEmpty())
        KMessageBox::information(this,
                                 enable ? i18n("You did not select anything to enable.")
                                        : i18n("You did not select anything to disable."),
                                 enable ? i18n("Nothing to Enable") : i18n("Nothing to Disable"));
    else
        toggleFonts(urls, fonts, enable, grp);
}

void CKCmFontInst::toggleFonts(CJobRunner::ItemList &urls, const QStringList &fonts, bool enable,
                               const QString &grp)
{
    bool doIt=false;

    switch(fonts.count())
    {
        case 0:
            break;
        case 1:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                       grp.isEmpty()
                            ? enable ? i18n("<p>Do you really want to "
                                            "enable</p><p>\'<b>%1</b>\'?</p>", fonts.first())
                                     : i18n("<p>Do you really want to "
                                            "disable</p><p>\'<b>%1</b>\'?</p>", fonts.first())
                            : enable ? i18n("<p>Do you really want to "
                                            "enable</p><p>\'<b>%1</b>\', "
                                            "contained within group \'<b>%2</b>\'?</p>",
                                            fonts.first(), grp)
                                     : i18n("<p>Do you really want to "
                                            "disable</p><p>\'<b>%1</b>\', "
                                            "contained within group \'<b>%2</b>\'?</p>",
                                            fonts.first(), grp),
                       enable ? i18n("Enable Font") : i18n("Disable Font"),
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable Font"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable Font")));
            break;
        default:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                       grp.isEmpty()
                            ? enable ? i18np("Do you really want to enable this font?",
                                             "Do you really want to enable these %1 fonts?",
                                             urls.count())
                                     : i18np("Do you really want to disable this font?",
                                             "Do you really want to disable these %1 fonts?",
                                             urls.count())
                            : enable ? i18np("<p>Do you really want to enable this font "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             "<p>Do you really want to enable these %1 fonts "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             urls.count(), grp)
                                     : i18np("<p>Do you really want to disable this font "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             "<p>Do you really want to disable these %1 fonts "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             urls.count(), grp),
                       fonts,
                       enable ? i18n("Enable Fonts") : i18n("Disable Fonts"),
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable Fonts"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable Fonts")));
    }

    if(doIt)
    {
        if(enable)
            itsStatusLabel->setText(i18n("Enabling font(s)..."));
        else
            itsStatusLabel->setText(i18n("Disabling font(s)..."));

        doCmd(enable ? CJobRunner::CMD_ENABLE : CJobRunner::CMD_DISABLE, urls);
    }
}

void CKCmFontInst::selectMainGroup()
{
    selectGroup(/*Misc::root()
                    ? */CGroupListItem::ALL/* : CGroupListItem::PERSONAL*/);
}

void CKCmFontInst::doCmd(CJobRunner::ECommand cmd, const CJobRunner::ItemList &urls, bool system)
{
    itsFontList->setSlowUpdates(true);
    CJobRunner runner(this);

    connect(&runner, SIGNAL(configuring()), itsFontList, SLOT(unsetSlowUpdates()));
    runner.exec(cmd, urls, system);
    itsFontList->setSlowUpdates(false);
    refreshFontList();
    if(CJobRunner::CMD_DELETE==cmd)
        itsFontListView->clearSelection();
    CFcEngine::setDirty();
    setStatusBar();
    delete itsTempDir;
    itsTempDir=NULL;
    itsFontListView->repaint();
    removeDeletedFontsFromGroups();
}

}

#include "KCmFontInst.moc"
