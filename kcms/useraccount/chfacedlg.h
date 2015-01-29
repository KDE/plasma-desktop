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
 * @file UserInfo-chface: Dialog for choosing a new face for your user.
 * @author Braden MacDonald
 */

#ifndef CHFACEDLG_H
#define CHFACEDLG_H

#include <QPixmap>

#include <KDialog>

#include "ui_faceDlg.h"

enum FacePerm { adminOnly = 1, adminFirst = 2, userFirst = 3, userOnly = 4};

class ChFaceDlg : public KDialog
{
  Q_OBJECT
public:


  explicit ChFaceDlg(const QString& picsdirs,
                     QWidget *parent=0);


  /**
   * Will return the currently selected face, or a null pixmap if the user hit the "remove image" button
   */
  QPixmap getFaceImage() const
  {
    if(ui.m_FacesWidget->currentItem())
      return ui.m_FacesWidget->currentItem()->icon().pixmap(64);
    else
      return QPixmap();
  }

private Q_SLOTS:
  void slotFaceWidgetSelectionChanged( QListWidgetItem *item )
    { enableButton( Ok, !item->icon().isNull() ); }

  void slotGetCustomImage();
  //void slotSaveCustomImage();
  void slotRemoveImage();

private:
  void addCustomPixmap( const QString &imPath, bool saveCopy );

  Ui::faceDlg ui;
};

#endif // CHFACEDLG_H
