/*
   Copyright (C) 2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include <KAboutData>
#include <QApplication>

#include <KLocalizedString>
#include <QCommandLineParser>

#include "knetattach.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("knetattach5");

    KAboutData aboutData(QLatin1String("knetattach"), i18n("KDE Network Wizard"), QLatin1String(PROJECT_VERSION), i18n("KDE Network Wizard"),KAboutLicense::GPL, i18n("(c) 2004 George Staikos"), QLatin1String("https://www.kde.org/"));

    aboutData.addAuthor(i18n("George Staikos"), i18n("Primary author and maintainer"), QStringLiteral("staikos@kde.org"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
	KNetAttach na;
    app.connect( &app, &QGuiApplication::lastWindowClosed, &app, &QCoreApplication::quit );
	na.show();

	return app.exec();
}
