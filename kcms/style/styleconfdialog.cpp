/*
 * KCMStyle's container dialog for custom style setup dialogs
 *
 * (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "styleconfdialog.h"
#include <KLocalizedString>

StyleConfigDialog::StyleConfigDialog(QWidget* parent, QString styleName)
  : KDialog( parent )
{
  setObjectName( "StyleConfigDialog" );
  setModal( true );
  setCaption( i18n( "Configure %1", styleName ) );
  setButtons( Default | Ok | Cancel );
  setDefaultButton( Cancel );

  m_dirty = false;
  connect( this, SIGNAL(defaultClicked()), this, SIGNAL(defaults()));
  connect( this, SIGNAL(okClicked()), this, SIGNAL(save()));
}

bool StyleConfigDialog::isDirty() const
{
  return m_dirty;
}

void StyleConfigDialog::setDirty(bool dirty)
{
  m_dirty = dirty;
}

