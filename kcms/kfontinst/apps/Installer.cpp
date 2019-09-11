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

#include "Installer.h"
#include "Misc.h"
#include "FontsPackage.h"
#include <QFile>
#include <QTemporaryDir>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <KAboutData>
#include <KMessageBox>
#include <KIO/StatJob>
#include "JobRunner.h"
#include "CreateParent.h"
#include "config-workspace.h"

namespace KFI
{

int CInstaller::install(const QSet<QUrl> &urls)
{
    QSet<QUrl>::ConstIterator it(urls.begin()),
                              end(urls.end());
    bool                      sysInstall(false);
    CJobRunner *jobRunner=new CJobRunner(itsParent);

    CJobRunner::startDbusService();

    if(!Misc::root())
    {
        switch(KMessageBox::questionYesNoCancel(itsParent,
                                       i18n("Do you wish to install the font(s) for personal use "
                                            "(only available to you), or "
                                            "system-wide (available to all users)?"),
                                       i18n("Where to Install"), KGuiItem(i18n(KFI_KIO_FONTS_USER)),
                                       KGuiItem(i18n(KFI_KIO_FONTS_SYS))))
        {
            case KMessageBox::No:
                sysInstall=true;
                break;
            case KMessageBox::Cancel:
                return -1;
            default:
                break;
        }
    }

    QSet<QUrl> instUrls;

    for(; it!=end; ++it)
    {
        auto job = KIO::mostLocalUrl(*it);
        job->exec();
        QUrl local = job->mostLocalUrl();
        bool package(false);

        if(local.isLocalFile())
        {
            QString localFile(local.toLocalFile());

            if(Misc::isPackage(localFile))
            {
                instUrls+=FontsPackage::extract(localFile, &itsTempDir);
                package=true;
            }
        }
        if(!package)
        {
            QList<QUrl> associatedUrls;

            CJobRunner::getAssociatedUrls(*it, associatedUrls, false, itsParent);
            instUrls.insert(*it);

            QList<QUrl>::Iterator aIt(associatedUrls.begin()),
                                 aEnd(associatedUrls.end());

            for(; aIt!=aEnd; ++aIt)
                instUrls.insert(*aIt);
        }
    }

    if(!instUrls.isEmpty())
    {
        CJobRunner::ItemList      list;
        QSet<QUrl>::ConstIterator it(instUrls.begin()),
                                  end(instUrls.end());

        for(; it!=end; ++it)
            list.append(*it);

        return jobRunner->exec(CJobRunner::CMD_INSTALL, list, Misc::root() || sysInstall);
    }
    else
        return -1;
}

CInstaller::~CInstaller()
{
    delete itsTempDir;
}

}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    KLocalizedString::setApplicationDomain(KFI_CATALOGUE);
    KAboutData aboutData("kfontinst", i18n("Font Installer"), WORKSPACE_VERSION_STRING, i18n("Simple font installer"),
                         KAboutLicense::GPL, i18n("(C) Craig Drummond, 2007"));
    KAboutData::setApplicationData(aboutData);

    QGuiApplication::setWindowIcon(QIcon::fromTheme("preferences-desktop-font-installer"));

    QCommandLineParser parser;
    const QCommandLineOption embedOption(QLatin1String("embed"), i18n("Makes the dialog transient for an X app specified by winid"), QLatin1String("winid"));
    parser.addOption(embedOption);
    parser.addPositionalArgument(QLatin1String("[URL]"), i18n("URL to install"));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QSet<QUrl> urls;

    foreach (const QString &arg, parser.positionalArguments())
        urls.insert(QUrl::fromUserInput(arg, QDir::currentPath()));

    if (!urls.isEmpty())
    {
        QString opt(parser.value(embedOption));
        KFI::CInstaller inst(createParent(opt.size() ? opt.toInt(nullptr, 16) : 0));

        return inst.install(urls);
    }

    return -1;
}
