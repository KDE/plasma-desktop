/**
 *  Copyright 2003 Braden MacDonald <bradenm_k@shaw.ca>                   
 *  Copyright 2003 Ravikiran Rajagopal <ravi@ee.eng.ohio-state.edu>       
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *
 *
 *  Please see the README
 *
 */

/**
 * @file UserInfo's Dialog for changing your face.
 * @author Braden MacDonald
 */

#include "chfacedlg.h"

#include <QPixmap>
#include <QImage>
#include <QPushButton>
#include <QDir>
#include <QCheckBox>

#include <klocale.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kio/filecopyjob.h>

#include "settings.h" // KConfigXT



/**
 * TODO: It would be nice if the widget were in a .ui
 */
ChFaceDlg::ChFaceDlg(const QString& picsdir, QWidget *parent)
  : KDialog( parent )
{
  setWindowTitle( i18nc("@title:window", "Change your Face") );
  setButtons( Ok|Cancel|User1|User2 );
  setDefaultButton( Ok );

  setButtonText( User1, i18n("Custom Image...") );
  setButtonText( User2, i18n("Remove Image") );

  QWidget *faceDlg = new QWidget;
  ui.setupUi(faceDlg);

  setMainWidget(faceDlg);

  connect(ui.m_FacesWidget, &QListWidget::currentItemChanged, this, &ChFaceDlg::slotFaceWidgetSelectionChanged);

  connect(ui.m_FacesWidget, &QListWidget::doubleClicked, this, &ChFaceDlg::accept);
  connect(this, &ChFaceDlg::okClicked, this, &ChFaceDlg::accept);

  connect(this, &ChFaceDlg::user1Clicked, this, &ChFaceDlg::slotGetCustomImage);

  connect(this, &ChFaceDlg::user2Clicked, this, &ChFaceDlg::slotRemoveImage);

#if 0
  QPushButton *acquireBtn = new QPushButton( i18n("&Acquire Image..."), page );
  acquireBtn->setEnabled( false );
  morePics->addWidget( acquireBtn );
#endif

  // Filling the icon view
  QDir facesDir( picsdir );
  if ( facesDir.exists() )
  {
    const QStringList picslist = facesDir.entryList( QDir::Files );
    for ( QStringList::const_iterator it = picslist.constBegin(); it != picslist.constEnd(); ++it )
      new QListWidgetItem( QIcon( picsdir + *it ), (*it).section('.',0,0), ui.m_FacesWidget );
  }
  facesDir.setPath( KCFGUserAccount::userFaceDir() );
  if ( facesDir.exists() )
  {
    const QStringList picslist = facesDir.entryList( QDir::Files );
    for ( QStringList::const_iterator it = picslist.constBegin(); it != picslist.constEnd(); ++it )
      new QListWidgetItem( QIcon( KCFGUserAccount::userFaceDir() + *it ),
                           QString('/'+(*it)) == KCFGUserAccount::customFaceFile() ? 
                           i18n("(Custom)") : (*it).section('.',0,0),
                           ui.m_FacesWidget );
  }


  enableButtonOk( false ); // since no item is pre-selected, we must only enable the Ok button once a selection is done!
  //connect( this, SIGNAL(okClicked()), SLOT(slotSaveCustomImage()) );

  resize( 420, 400 );
}

void ChFaceDlg::addCustomPixmap( const QString &imPath, bool saveCopy )
{
  QImage pix( imPath );
  // TODO: save pix to TMPDIR/userinfo-tmp,
  // then scale and copy *that* to ~/.faces

  if (pix.isNull())
  {
    KMessageBox::sorry( this, i18n("There was an error loading the image.") );
    return;
  }
  if ( (pix.width() > KCFGUserAccount::faceSize())
	|| (pix.height() > KCFGUserAccount::faceSize()) )
    pix = pix.scaled( KCFGUserAccount::faceSize(), KCFGUserAccount::faceSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation );// Should be no bigger than certain size.

  if ( saveCopy )
  {
    // If we should save a copy:
    QDir userfaces( KCFGUserAccount::userFaceDir() );
    if ( !userfaces.exists( ) )
      userfaces.mkdir( userfaces.absolutePath() );

    pix.save( userfaces.absolutePath() + "/.userinfo-tmp" , "PNG" );
    auto job = KIO::file_copy(KUrl(userfaces.absolutePath() + "/.userinfo-tmp"),
                              KUrl(userfaces.absolutePath() + '/' + QFileInfo(imPath).fileName().section('.', 0, 0)));
    job->exec();
#if 0
  if ( !pix.save( userfaces.absolutePath() + '/' + imPath , "PNG" ) )
    KMessageBox::sorry(this, i18n("There was an error saving the image:\n%1", userfaces.absolutePath() ) );
#endif
  }

  QListWidgetItem* newface = new QListWidgetItem( QIcon(QPixmap::fromImage(pix)), QFileInfo(imPath).fileName().section('.',0,0), ui.m_FacesWidget );
  ui.m_FacesWidget->scrollToItem( newface );
  ui.m_FacesWidget->setCurrentItem( newface );
}

void ChFaceDlg::slotGetCustomImage(  )
{
  QCheckBox* checkWidget = new QCheckBox( i18n("&Save copy in custom faces folder for future use"), 0 );

  KFileDialog dlg( QDir::homePath(), KImageIO::pattern( KImageIO::Reading ),
                  this, checkWidget);

  dlg.setOperationMode( KFileDialog::Opening );
  dlg.setWindowTitle( i18nc("@title:window", "Choose Image") );
  dlg.setMode( KFile::File | KFile::LocalOnly );

  KImageFilePreview *ip = new KImageFilePreview( &dlg );
  dlg.setPreviewWidget( ip );
  if (dlg.exec() == QDialog::Accepted)
      addCustomPixmap( dlg.selectedFile(), checkWidget->isChecked() );
}

void ChFaceDlg::slotRemoveImage()
{
  ui.m_FacesWidget->clearSelection();
  accept();
}

#if 0
void ChFaceDlg::slotSaveCustomImage()
{
  if ( m_FacesWidget->currentItem()->key() ==  USER_CUSTOM_KEY)
  {
    QDir userfaces( QDir::homePath() + USER_FACES_DIR );
    if ( !userfaces.exists( ) )
      userfaces.mkdir( userfaces.absolutePath() );

    if ( !m_FacesWidget->currentItem()->pixmap()->save( userfaces.absolutePath() + USER_CUSTOM_FILE , "PNG" ) )
      KMessageBox::sorry(this, i18n("There was an error saving the image:\n%1", userfaces.absolutePath() ) );
  }
}
#endif

