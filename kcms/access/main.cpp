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
#include <kdebug.h>
#include <k4aboutdata.h>
#include <kcmdlineargs.h>
#include <KLocalizedString>
#include <QX11Info>
#include <Kdelibs4ConfigMigrator>

extern "C" Q_DECL_EXPORT int kdemain(int argc, char * argv[] )
{
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kaccess"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kaccessrc"));
    migrate.migrate();

  K4AboutData about(I18N_NOOP("kaccess"), 0, ki18n("KDE Accessibility Tool"),
                  0, KLocalizedString(), K4AboutData::License_GPL,
                  ki18n("(c) 2000, Matthias Hoelzer-Kluepfel"));

  about.addAuthor(ki18n("Matthias Hoelzer-Kluepfel"), ki18n("Author") , "hoelzer@kde.org");

  KCmdLineArgs::init( argc, argv, &about );

  if (!KAccessApp::start())
    return 0;

  // verify the Xlib has matching XKB extension
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  if (!XkbLibraryVersion(&major, &minor))
    {
      kError() << "Xlib XKB extension does not match" << endl;
      return 1;
    }
  kDebug() << "Xlib XKB extension major=" << major << " minor=" << minor;

  // we need an application object for QX11Info
  KAccessApp app;

  // verify the X server has matching XKB extension
  // if yes, the XKB extension is initialized
  int opcode_rtrn;
  int error_rtrn;
  int xkb_opcode;
  if (!XkbQueryExtension(QX11Info::display(), &opcode_rtrn, &xkb_opcode, &error_rtrn,
			 &major, &minor))
    {
      kError() << "X server has not matching XKB extension" << endl;
      return 1;
    }
  kDebug() << "X server XKB extension major=" << major << " minor=" << minor;

  //Without that, the application dies when the dialog is closed only once.
  app.setQuitOnLastWindowClosed(false);
  
  app.setXkbOpcode(xkb_opcode);
  app.disableSessionManagement();
  return app.exec();
}
