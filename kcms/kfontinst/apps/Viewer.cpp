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

#include "config-workspace.h"
#include "Viewer.h"
#include "KfiConstants.h"
#include <KCmdLineArgs>
#include <k4aboutdata.h>
#include <KUniqueApplication>
#include <KPluginLoader>
#include <KPluginFactory>
#include <KFileDialog>
#include <KStandardAction>
#include <KActionCollection>
#include <KShortcutsDialog>
#include <KConfigGroup>
#include <KParts/BrowserExtension>
#include <QUrl>
#include <QAction>

namespace KFI
{

CViewer::CViewer()
{
    KPluginFactory *factory=KPluginLoader("kfontviewpart").factory();

    if(factory)
    {
        itsPreview=factory->create<KParts::ReadOnlyPart>(this);

        actionCollection()->addAction(KStandardAction::Open, this, SLOT(fileOpen()));
        actionCollection()->addAction(KStandardAction::Quit, this, SLOT(close()));
        actionCollection()->addAction(KStandardAction::KeyBindings, this, SLOT(configureKeys()));
        itsPrintAct=actionCollection()->addAction(KStandardAction::Print, itsPreview, SLOT(print()));

        itsPrintAct->setEnabled(false);

        if(itsPreview->browserExtension())
            connect(itsPreview->browserExtension(), SIGNAL(enableAction(const char*,bool)),
                    this, SLOT(enableAction(const char*,bool)));

        setCentralWidget(itsPreview->widget());
        createGUI(itsPreview);

        setAutoSaveSettings();
        applyMainWindowSettings(KSharedConfig::openConfig()->group("MainWindow"));
    }
    else
        exit(0);
}

void CViewer::fileOpen()
{
    QUrl url(KFileDialog::getOpenUrl(QUrl(), "application/x-font-ttf application/x-font-otf "
                                             "application/x-font-type1 "
                                             "application/x-font-bdf application/x-font-pcf ",
                                     this, i18n("Select Font to View")));
    showUrl(url);
}

void CViewer::showUrl(const QUrl &url)
{
    if(url.isValid())
        itsPreview->openUrl(url);
}

void CViewer::configureKeys()
{
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);

    dlg.addCollection(actionCollection());
    dlg.configure();
}

void CViewer::enableAction(const char *name, bool enable)
{
    if(0==qstrcmp("print", name))
        itsPrintAct->setEnabled(enable);
}




class ViewerApplication : public KUniqueApplication
{
    public:

#ifdef Q_WS_X11
    ViewerApplication(Display *display, Qt::HANDLE visual, Qt::HANDLE colormap)
        : KUniqueApplication(display,visual,colormap)
    {
    }
#endif

    ViewerApplication() : KUniqueApplication()
    {
    }

    int newInstance()
    {
        KCmdLineArgs *args(KCmdLineArgs::parsedArgs());
        KFI::CViewer *viewer=new KFI::CViewer;

        viewer->show();
        if(args->count() > 0)
        {
            for (int i = 0; i < args->count(); ++i)
            {
                QUrl url(args->url(i));

                if (i != 0)
                {
                    viewer=new KFI::CViewer;
                    viewer->show();
                }
                viewer->showUrl(url);
            }
        }

        return 0;
    }
};

}

static K4AboutData aboutData("kfontview", KFI_CATALOGUE, ki18n("Font Viewer"), WORKSPACE_VERSION_STRING, ki18n("Simple font viewer"),
                             K4AboutData::License_GPL, ki18n("(C) Craig Drummond, 2004-2007"));

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addTempFileOption();

    KCmdLineOptions options;
    options.add("+[URL]", ki18n("URL to open"));
    KCmdLineArgs::addCmdLineOptions(options);

    if (!KUniqueApplication::start())
        exit(0);

    KFI::ViewerApplication app;

    return app.exec();
}

#include "Viewer.moc"
