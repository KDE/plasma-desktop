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
#include <QDrag>
#include <QMimeData>

#include "installer.h"

#include <kdebug.h>
#include <kfiledialog.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
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
//  KGlobal::dirs()->addResourceType("ksplashthemes", "data", "ksplash/Themes");

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
  leftbox->addWidget(mThemesList);

  mBtnTest = new KPushButton( QIcon::fromTheme("document-preview"), i18n("Test Theme"), this );
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


//-----------------------------------------------------------------------------
void SplashInstaller::readThemesList()
{
  mThemesList->clear();

  // Read local themes
//TODO
//  const QStringList entryList = KGlobal::dirs()->resourceDirs("ksplashthemes");
  QStringList entryList;
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
