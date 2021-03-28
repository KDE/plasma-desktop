/*
    Copyright 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    Copyright 2014 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kaccess.h"

#include <KAboutData>
#include <KLocalizedString>
#include <Kdelibs4ConfigMigrator>
#include <QApplication>
#include <QDebug>
#include <QX11Info>

int main(int argc, char *argv[])
{
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kaccess"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kaccessrc"));
    migrate.migrate();

    qunsetenv("SESSION_MANAGER");

    // this application is currently only relevant on X, force to run under X
    // note if someone does port this we still need to run kaccess under X for xwayland apps
    qputenv("QT_QPA_PLATFORM", "xcb");

    // verify the Xlib has matching XKB extension
    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;
    if (!XkbLibraryVersion(&major, &minor)) {
        qWarning() << "Xlib XKB extension does not match";
        return 1;
    }
    qDebug() << "Xlib XKB extension major=" << major << " minor=" << minor;

    // we need an application object for QX11Info
    QApplication app(argc, argv);

    KAboutData about(QStringLiteral("kaccess"), QString(), i18n("Accessibility"), {}, KAboutLicense::GPL_V2, i18n("(c) 2000, Matthias Hoelzer-Kluepfel"));

    about.addAuthor(i18n("Matthias Hoelzer-Kluepfel"), i18n("Author"), QStringLiteral("hoelzer@kde.org"));
    // set data as used for D-Bus by KAccessApp
    KAboutData::setApplicationData(about);

    KAccessApp acc;
    if (acc.isFailed()) {
        return 1;
    }

    // verify the X server has matching XKB extension
    // if yes, the XKB extension is initialized
    int opcode_rtrn;
    int error_rtrn;
    int xkb_opcode;
    if (!XkbQueryExtension(QX11Info::display(), &opcode_rtrn, &xkb_opcode, &error_rtrn, &major, &minor)) {
        qWarning() << "X server has not matching XKB extension" << Qt::endl;
        return 1;
    }
    qDebug() << "X server XKB extension major=" << major << " minor=" << minor;

    app.installNativeEventFilter(&acc);

    // Without that, the application dies when the dialog is closed only once.
    app.setQuitOnLastWindowClosed(false);

    acc.setXkbOpcode(xkb_opcode);
    return app.exec();
}
