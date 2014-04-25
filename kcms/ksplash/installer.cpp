/***************************************************************************
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *   Copyright (c) 1998 Stefan Taferner <taferner@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <stdlib.h>

#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QTextEdit>
#include <QPixmap>
#include <QFrame>
#include <QHBoxLayout>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QScrollArea>

#include "installer.h"

#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kservicetypetrader.h>
#include <kio/netaccess.h>
#include <KNewStuff3/KNS3/DownloadDialog>

ThemeListBox::ThemeListBox(QWidget *parent)
  : KListWidget(parent)
{
   setAcceptDrops(true);
}

void ThemeListBox::dragEnterEvent(QDragEnterEvent* event)
{
   event->setAccepted((event->source() != this) && KUrl::List::canDecode(event->mimeData()));
}

void ThemeListBox::dragMoveEvent(QDragMoveEvent* event)
{
   event->setAccepted((event->source() != this) && KUrl::List::canDecode(event->mimeData()));
}

void ThemeListBox::dropEvent(QDropEvent* event)
{
   KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
   if (!urls.isEmpty())
   {
      emit filesDropped(urls);
   }
}

void ThemeListBox::mousePressEvent(QMouseEvent *e)
{
   if ((e->buttons() & Qt::LeftButton) != 0)
   {
      mOldPos = e->globalPos();;
      mDragFile.clear();
      int cur = row(itemAt(e->pos()));
      if (cur >= 0)
         mDragFile = text2path[item(cur)->text()];
   }
   KListWidget::mousePressEvent(e);
}

void ThemeListBox::mouseMoveEvent(QMouseEvent *e)
{
   if (((e->buttons() & Qt::LeftButton) != 0) && !mDragFile.isEmpty())
   {
      int delay = KGlobalSettings::dndEventDelay();
      QPoint newPos = e->globalPos();
      if(newPos.x() > mOldPos.x()+delay || newPos.x() < mOldPos.x()-delay ||
         newPos.y() > mOldPos.y()+delay || newPos.y() < mOldPos.y()-delay)
      {
         KUrl url;
         url.setPath(mDragFile);
         KUrl::List urls;
         urls.append(url);
         QDrag *drag = new QDrag(this);
         QMimeData *mime = new QMimeData();
         urls.populateMimeData(mime);
         drag->setMimeData(mime);
         drag->start();
      }
   }
   KListWidget::mouseMoveEvent(e);
}

//-----------------------------------------------------------------------------
SplashInstaller::SplashInstaller (QWidget *aParent, const char *aName, bool aInit)
  : QWidget(aParent), mGui(!aInit)
{
  setObjectName(aName);
  KGlobal::dirs()->addResourceType("ksplashthemes", "data", "ksplash/Themes");

  if (!mGui)
    return;

  QHBoxLayout* hbox = new QHBoxLayout( this );
  hbox->setMargin( 0 );

  QVBoxLayout* leftbox = new QVBoxLayout(  );
  hbox->addLayout( leftbox );
  hbox->setStretchFactor( leftbox, 1 );

  mThemesList = new ThemeListBox(this);
  mThemesList->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
  connect(mThemesList, SIGNAL(currentRowChanged(int)), SLOT(slotSetTheme(int)));
  connect(mThemesList, SIGNAL(filesDropped(KUrl::List)), SLOT(slotFilesDropped(KUrl::List)));
  leftbox->addWidget(mThemesList);

  mBtnNew = new KPushButton( KIcon("get-hot-new-stuff"), i18n("Get New Themes..."), this );
  mBtnNew->setToolTip(i18n("Get new themes from the Internet"));
  mBtnNew->setWhatsThis(i18n("You need to be connected to the Internet to use this action. A dialog will display a list of themes from the http://www.kde.org website. Clicking the Install button associated with a theme will install this theme locally."));
  leftbox->addWidget( mBtnNew );
  connect(mBtnNew, SIGNAL(clicked()), SLOT(slotNew()));

  mBtnAdd = new KPushButton( KIcon("document-import"), i18n("Install Theme File..."), this );
  mBtnAdd->setToolTip(i18n("Install a theme archive file you already have locally"));
  mBtnAdd->setWhatsThis(i18n("If you already have a theme archive locally, this button will unpack it and make it available for KDE applications"));
  leftbox->addWidget( mBtnAdd );
  connect(mBtnAdd, SIGNAL(clicked()), SLOT(slotAdd()));

  mBtnRemove = new KPushButton( KIcon("edit-delete"), i18n("Remove Theme"), this );
  mBtnRemove->setToolTip(i18n("Remove the selected theme from your disk"));
  mBtnRemove->setWhatsThis(i18n("This will remove the selected theme from your disk."));
  mBtnRemove->setEnabled( false );
  leftbox->addWidget( mBtnRemove );
  connect(mBtnRemove, SIGNAL(clicked()), SLOT(slotRemove()));

  mBtnTest = new KPushButton( KIcon("document-preview"), i18n("Test Theme"), this );
  mBtnTest->setToolTip(i18n("Test the selected theme"));
  mBtnTest->setWhatsThis(i18n("This will test the selected theme."));
  mBtnTest->setEnabled( false );
  leftbox->addWidget( mBtnTest );
  connect(mBtnTest, SIGNAL(clicked()), SLOT(slotTest()));

  QVBoxLayout* rightbox = new QVBoxLayout(  );
  hbox->addLayout( rightbox );
  hbox->setStretchFactor( rightbox, 3 );

  QScrollArea* scrollarea = new QScrollArea(this);
  scrollarea->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  mPreview = new QLabel(this);
  scrollarea->setWidget(mPreview);
  mPreview->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  mPreview->setMinimumSize(QSize(320,240));
  mPreview->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  rightbox->addWidget(scrollarea);
  rightbox->setStretchFactor( scrollarea, 3 );

  mText = new QTextEdit(this);
  mText->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  mText->setMinimumSize(mText->sizeHint().width(), 7 * mText->fontMetrics().height());
  mText->setReadOnly(true);
  rightbox->addWidget(mText);
  rightbox->setStretchFactor( mText, 1 );


  readThemesList();
  load();
}


//-----------------------------------------------------------------------------
SplashInstaller::~SplashInstaller()
{
}

int SplashInstaller::addTheme(const QString &path, const QString &name)
{
  qDebug() << "SplashInstaller::addTheme: " << path << " " << name;
  QString tmp(i18n( name.toUtf8() ));
  int i = mThemesList->count();
  while((i > 0) && (mThemesList->item(i-1)->text() > tmp))
    i--;
  if ((i > 0) && (mThemesList->item(i-1)->text() == tmp))
    return i-1;
  mThemesList->insertItem(i , tmp);
  mThemesList->text2path.insert( tmp, path+'/'+name );
  return i;
}

// Copy theme package into themes directory
void SplashInstaller::addNewTheme(const KUrl &srcURL)
{
  const QString dir = KGlobal::dirs()->saveLocation("ksplashthemes");

  KUrl url;
  QStringList themeNames;
  QString filename = srcURL.fileName();
  int i = filename.lastIndexOf('.');
  // Convert extension to lower case.
  if (i >= 0)
     filename = filename.left(i)+filename.mid(i).toLower();
  url.setPath(KStandardDirs::locateLocal("tmp",filename));

  // Remove file from temporary directory if it aleady exists - usually the result of a failed install.
  if ( KIO::NetAccess::exists( url, KIO::NetAccess::SourceSide, 0 ) )
    KIO::NetAccess::del( url, 0 );

  if (srcURL.fileName().toLower() == "theme.rc" ) // uncompressed theme selected
  {
    QString themeName;
    // Get the name of the Theme from the theme.rc file
    KConfig _cnf(srcURL.path());
    KConfigGroup cnf(&_cnf, QString("KSplash Theme: %1").arg(themeName) );

    // copy directory of theme.rc to ThemesDir
    KIO::NetAccess::dircopy(KUrl(srcURL.directory()), KUrl(dir + themeName));

    themeNames << themeName;
  }
  else
  {
    bool rc = KIO::NetAccess::file_copy(srcURL, url, 0);
    if (!rc)
    {
      kWarning() << "Failed to copy theme " << srcURL.fileName()
	  << " into temporary directory " << url.path() << endl;
      return;
    }

    // Extract into theme directory: we may have multiple themes in one tarball!
    KTar tarFile(url.path());
    if (!tarFile.open(QIODevice::ReadOnly))
    {
      kWarning() << "Unable to open archive: " << url.path();
      KIO::NetAccess::del( url, 0 );
      return;
    }
    KArchiveDirectory const *ad = tarFile.directory();

    // Find first directory entry.
    const QStringList entries = ad->entries();
    foreach(const QString& s, entries)
    {
      if( ad->entry(s)->isDirectory() )
      {
	// each directory may contain one theme
	themeNames << s;
      }
    }
    if (themeNames.count() < 1)
    {
      kWarning() << "No directory in archive: " << url.path();
      tarFile.close();
      KIO::NetAccess::del( url, 0 );
      return;
    }

    // copy the theme into the "ksplashthemes" directory
    ad->copyTo(dir);

    tarFile.close();
    KIO::NetAccess::del( url, 0 );

  }

  readThemesList();
  mThemesList->setCurrentRow(findTheme(themeNames.first()));
  if (mThemesList->currentItem())
  {
    mThemesList->currentItem()->setSelected(true);
  }
}

//-----------------------------------------------------------------------------
void SplashInstaller::readThemesList()
{
  mThemesList->clear();

  // Read local themes
  const QStringList entryList = KGlobal::dirs()->resourceDirs("ksplashthemes");
  //kDebug() << "readThemesList: " << entryList;
  QDir dir;
  QStringList subdirs;
  QStringList::ConstIterator name;
  for(name = entryList.constBegin(); name != entryList.constEnd(); name++)
  {
    dir = *name;
    if (!dir.exists())
      continue;
    subdirs = dir.entryList( QDir::Dirs );
    // kDebug() << "readThemesList: " << subdirs;
    // TODO: Make sure it contains a *.rc file.
    for (QStringList::const_iterator l = subdirs.constBegin(); l != subdirs.constEnd(); l++ )
      if ( !(*l).startsWith(QString(".")) )
      {
        mThemesList->blockSignals( true ); // Don't activate any theme until all themes are loaded.
        addTheme(dir.path(),*l);
        mThemesList->blockSignals( false );
      }
  }
}

//-----------------------------------------------------------------------------
void SplashInstaller::defaults()
{
  mThemesList->setCurrentRow(findTheme("Default"));
  emit changed( true );
}

void SplashInstaller::load()
{
  KConfig _cnf( "ksplashrc" );
  KConfigGroup cnf(&_cnf, "KSplash");
  QString curTheme = cnf.readEntry("Theme","Default");
  mThemesList->setCurrentRow(findTheme(curTheme));
  emit changed( false );
}

//-----------------------------------------------------------------------------
void SplashInstaller::save()
{
  KConfig _cnf( "ksplashrc" );
  KConfigGroup cnf(&_cnf, "KSplash");
  int cur = mThemesList->currentRow();
  if (cur < 0)
    return;
  QString path = mThemesList->item(cur)->text();
  if ( mThemesList->text2path.contains( path ) )
    path = mThemesList->text2path[path];
  cur = path.lastIndexOf('/');
  cnf.writeEntry("Theme", path.mid(cur+1) );
  // save also the engine, so that it's known at KDE startup which splash implementation to use
  cnf.writeEntry("Engine", mEngineOfSelected );
  cnf.sync();
  emit changed( false );
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotRemove()
{
  int cur = mThemesList->currentRow();
  if (cur < 0)
    return;

  bool rc = false;
  const QString themeName = mThemesList->item(cur)->text();
  const QString themeDir = mThemesList->text2path[themeName];
  if (!themeDir.isEmpty())
  {
     KUrl url;
     url.setPath(themeDir);
     if (KMessageBox::warningContinueCancel(this,i18n("Delete folder %1 and its contents?", themeDir),"",KGuiItem(i18n("&Delete"),"edit-delete"))==KMessageBox::Continue)
       rc = KIO::NetAccess::del(url,this);
     else
       return;
  }
  if (!rc)
  {
    KMessageBox::sorry(this, i18n("Failed to remove theme '%1'", themeName));
    return;
  }
  //mThemesList->removeItem(cur);
  readThemesList();
  cur = (cur >= mThemesList->count())?mThemesList->count()-1:cur;
  mThemesList->setCurrentRow(cur);
}


//-----------------------------------------------------------------------------
void SplashInstaller::slotSetTheme(int id)
{
  bool enabled;
  QString path = QString();
  QString infoTxt;

  if (id < 0)
  {
    mPreview->setText(QString());
    mText->setText(QString());
    enabled = false;
  }
  else
  {
    QString error = i18n("(Could not load theme)");
    path = mThemesList->item(id)->text();
    if ( mThemesList->text2path.contains( path ) )
        path = mThemesList->text2path[path];
    enabled = false;
    KUrl url;
    QString themeName;
    if (!path.isEmpty())
    {
      // Make sure the correct plugin is installed.
      int i = path.lastIndexOf('/');
      if (i >= 0)
        themeName = path.mid(i+1);
      url.setPath( path + "/Theme.rc" );
      if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0))
      {
        url.setPath( path + "/Theme.RC" );
        if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0))
        {
          url.setPath( path + "/theme.rc" );
          if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0))
            url.setPath( path + '/' + themeName + ".rc" );
        }
      }
      if (KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0))
      {
        KConfig _cnf(url.path());
	KConfigGroup cnf(&_cnf, QString("KSplash Theme: %1").arg(themeName) );

        // Get theme information.
        infoTxt = "<qt>";
        if ( cnf.hasKey( "Name" ) )
          infoTxt += i18n( "<b>Name:</b> %1", cnf.readEntry( "Name", i18nc( "Unknown name", "Unknown" ) ) ) + "<br />";
        if ( cnf.hasKey( "Description" ) )
          infoTxt += i18n( "<b>Description:</b> %1", cnf.readEntry( "Description", i18nc( "Unknown description", "Unknown" ) ) ) + "<br />";
        if ( cnf.hasKey( "Version" ) )
          infoTxt += i18n( "<b>Version:</b> %1", cnf.readEntry( "Version", i18nc( "Unknown version", "Unknown" ) ) ) + "<br />";
        if ( cnf.hasKey( "Author" ) )
          infoTxt += i18n( "<b>Author:</b> %1", cnf.readEntry( "Author", i18nc( "Unknown author", "Unknown" ) ) ) + "<br />";
        if ( cnf.hasKey( "Homepage" ) )
          infoTxt += i18n( "<b>Homepage:</b> %1", cnf.readEntry( "Homepage", i18nc( "Unknown homepage", "Unknown" ) ) ) + "<br />";
        infoTxt += "</qt>";

        QString pluginName( cnf.readEntry( "Engine", "KSplashX" ).trimmed() );
        if( pluginName == "Simple"
                || pluginName == "None"
                || pluginName == "KSplashX"
                || pluginName == "KSplashQML"
                )
            enabled = true; // these are not plugins
        else if ((KServiceTypeTrader::self()->query("KSplash/Plugin", QString("[X-KSplash-PluginName] == '%1'").arg(pluginName))).isEmpty())
        {
          enabled = false;
          error = i18n("This theme requires the plugin %1 which is not installed.", pluginName);
        }
        else
          enabled = true; // Hooray, there is at least one plugin which can handle this theme.
        mEngineOfSelected = pluginName;
      }
      else
      {
        error = i18n("Could not load theme configuration file.");
      }
    }
    mBtnTest->setEnabled(enabled && themeName != "None" );
    mText->setHtml(infoTxt);
    if (!enabled)
    {
      url.setPath( path + '/' + "Preview.png" );
      if (KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0))
        mPreview->setPixmap(QPixmap(url.path()));
      else
        mPreview->setText(i18n("(Could not load theme)"));
      KMessageBox::sorry(this, error);
    }
    else
    {
      url.setPath( path + '/' + "Preview.png" );
      if (KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0))
        mPreview->setPixmap(QPixmap(url.path()));
      else
        mPreview->setText(i18n("No preview available."));
      emit changed(true);
    }
  }
  mBtnRemove->setEnabled( !path.isEmpty() && QFileInfo(path).isWritable());
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotNew()
{
  KNS3::DownloadDialog dialog("ksplash.knsrc", this);
  dialog.exec();
  if (!dialog.changedEntries().isEmpty())
    readThemesList();
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotAdd()
{
  static QString path;
  if (path.isEmpty()) path = QDir::homePath();

  KFileDialog dlg(path, "*.tgz *.tar.gz *.tar.bz2 theme.rc|" + i18n( "KSplash Theme Files" ), this);
  dlg.setCaption(i18n("Add Theme"));
  if (!dlg.exec())
    return;

  path = dlg.baseUrl().url();
  addNewTheme(dlg.selectedUrl());
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotFilesDropped(const KUrl::List &urls)
{
  for(KUrl::List::ConstIterator it = urls.constBegin();
      it != urls.end();
      ++it)
      addNewTheme(*it);
}

//-----------------------------------------------------------------------------
int SplashInstaller::findTheme( const QString &theme )
{
  // theme is untranslated, but the listbox contains translated items
  QString tmp(i18n( theme.toUtf8() ));
  int id = mThemesList->count()-1;

  while (id >= 0)
  {
    if (mThemesList->item(id)->text() == tmp)
      return id;
    id--;
  }

  return 0;
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotTest()
{
  int i = mThemesList->currentRow();
  if (i < 0)
    return;
  QString themeName = mThemesList->text2path[mThemesList->item(i)->text()];
  int r = themeName.lastIndexOf('/');
  if (r >= 0)
    themeName = themeName.mid(r+1);

  // special handling for none and simple splashscreens
  qDebug() << "the engine is " << mEngineOfSelected << "for" << themeName;
  if( mEngineOfSelected == "None" )
    return;
  else if( mEngineOfSelected == "Simple" )
  {
    KProcess proc;
    proc << "ksplashsimple" << themeName << "--test";
    if (proc.execute())
      KMessageBox::error(this,i18n("Failed to successfully test the splash screen."));
    return;
  }
  else if( mEngineOfSelected == "KSplashX" )
  {
    KProcess proc;
    proc << "ksplashx" << themeName << "--test";
    if (proc.execute())
      KMessageBox::error(this,i18n("Failed to successfully test the splash screen."));
    return;
  }
  else if( mEngineOfSelected == "KSplashQML" )
  {
    KProcess proc;
    proc << "ksplashqml" << themeName << "--test";
    if (proc.execute())
      KMessageBox::error(this,i18n("Failed to successfully test the splash screen."));
    return;
  }
  else // KSplashML engines
  {
    KProcess proc;
    proc << "ksplash" << "--test" << "--theme" << themeName;
    if (proc.execute())
      KMessageBox::error(this,i18n("Failed to successfully test the splash screen."));
  }
}

//-----------------------------------------------------------------------------
#include "installer.moc"
