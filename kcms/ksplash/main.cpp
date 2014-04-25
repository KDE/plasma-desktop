/***************************************************************************
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/

#include <config-workspace.h>

#include <QLayout>
#include <QTabWidget>
//Added by qt3to4:
#include <QHBoxLayout>

#include <kaboutdata.h>
#include <kcmodule.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "installer.h"
#include <KPluginFactory>
#include <KPluginLoader>

class KSplashThemeMgr : public KCModule
{
    Q_OBJECT
public:
  KSplashThemeMgr( QWidget *parent, const QVariantList &/*unused*/);
  ~KSplashThemeMgr();

  QString quickHelp() const;

  virtual void init();
  virtual void save();
  virtual void load();
  virtual void defaults();

private:
  SplashInstaller *mInstaller;
};

K_PLUGIN_FACTORY(KSplashThemeMgrFactory,
        registerPlugin< KSplashThemeMgr>();
        )
K_EXPORT_PLUGIN(KSplashThemeMgrFactory("ksplashthemes"))

// -----------------------------------------------------------------------------------------

KSplashThemeMgr::KSplashThemeMgr( QWidget *parent, const QVariantList &args)
  : KCModule( KSplashThemeMgrFactory::componentData(), parent, args ), mInstaller(new SplashInstaller(this))
{
  init();

#if 0
  QHBoxLayout *box = new QHBoxLayout(this);
  QTabWidget *tab = new QTabWidget(this); // There will be more tabs in the future.
  box->addWidget(tab);
  tab->addTab( mInstaller, i18n("&Theme Installer") );
#else
  QHBoxLayout *box = new QHBoxLayout(this);
  box->setMargin(0);
  box->addWidget(mInstaller);
#endif
  connect( mInstaller, SIGNAL(changed(bool)), SIGNAL(changed(bool)) );
  KAboutData *about = new KAboutData( "kcmksplash"
                                      , 0,ki18n("KDE splash screen theme manager")
                                      ,"0.1"
                                      ,KLocalizedString()
                                      ,KAboutData::License_GPL
                                      ,ki18n("(c) 2003 KDE developers") );
  about->addAuthor(ki18n("Ravikiran Rajagopal"), KLocalizedString(), "ravi@ee.eng.ohio-state.edu");
  about->addCredit(ki18n("Brian Ledbetter"), ki18n("Original KSplash/ML author"), "brian@shadowcom.net");
  about->addCredit(ki18n("KDE Theme Manager authors" ), ki18n("Original installer code") );
  // Once string freeze is over, replace second argument with "Icon"
  about->addCredit(ki18n("Hans Karlsson"), KLocalizedString(), "karlsson.h@home.se" );
  setAboutData(about);
  //setButtons( KCModule::Default|KCModule::Apply );
}

KSplashThemeMgr::~KSplashThemeMgr()
{
  // Do not delete the installer as it is now owned by the tab widget.
}

QString KSplashThemeMgr::quickHelp() const
{
  return i18n("<h1>Splash Screen Theme Manager </h1> Install and view splash screen themes.");
}

void KSplashThemeMgr::init()
{
  KGlobal::dirs()->addResourceType("ksplashthemes", "data", "ksplash/Themes");
}

void KSplashThemeMgr::save()
{
  mInstaller->save();
}

void KSplashThemeMgr::load()
{
  mInstaller->load();
}

void KSplashThemeMgr::defaults()
{
  mInstaller->defaults();
}

#include "main.moc"
