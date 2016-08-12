/* KDE Display scheme editor
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "scmeditordialog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>

#include <KAboutData>

int main(int argc, char* argv[])
{

    QApplication app(argc, argv);

    KAboutData aboutData(
        QStringLiteral("kcolorschemeeditor"),
        i18n("KColorSchemeEditor"),
        QStringLiteral("0.1"),
        i18n("Utility to edit and create color schemes"),
        KAboutLicense::GPL_V3
    );
    aboutData.addAuthor(i18n("Olivier Churlaud"), i18n("Utility creation"),
                        QStringLiteral("olivier@churlaud.com"));
    aboutData.addAuthor(i18n("Jeremy Whiting"), i18n("KCM code (reused in here)"),
                        QStringLiteral("jpwhiting@kde.org"));
    aboutData.addAuthor(i18n("Matthew Woehlke"), i18n("KCM code (reused in here)"),
                        QStringLiteral("mw_triad@users.sourceforge.net"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("theme", i18n("Scheme to edit or to use as a base."),
        QStringLiteral("kcolorschemeeditor ThemeName"));
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    const QStringList args = parser.positionalArguments();
    QString path = "";
    if (args.count() == 1)
    {
        const QString fileBaseName(args.at(0));
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                "color-schemes/" + fileBaseName + ".colors");
    }
    if (path.isEmpty())
    {
        QTextStream out(stderr);
        out << i18n("Scheme not found, falling back to current one.\n");
    }

    SchemeEditorDialog dialog(path);

    dialog.show();

    return app.exec();
}
