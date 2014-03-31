
#include "kaccess.h"
#include <kdebug.h>
#include <k4aboutdata.h>
#include <kcmdlineargs.h>
#include <KLocalizedString>
#include <kdemacros.h>
#include <QX11Info>

extern "C" Q_DECL_EXPORT int kdemain(int argc, char * argv[] )
{
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
