/*
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    KAboutData aboutData(QLatin1String("knetattach"),
                         i18n("KDE Network Wizard"),
                         QLatin1String(PROJECT_VERSION),
                         i18n("KDE Network Wizard"),
                         KAboutLicense::GPL,
                         i18n("(c) 2004 George Staikos"),
                         QLatin1String("https://www.kde.org/"));

    aboutData.addAuthor(i18n("George Staikos"), i18n("Primary author and maintainer"), QStringLiteral("staikos@kde.org"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    KNetAttach na;
    app.connect(&app, &QGuiApplication::lastWindowClosed, &app, &QCoreApplication::quit);
    na.show();

    return app.exec();
}
