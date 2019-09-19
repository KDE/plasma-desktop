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
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

StyleConfigDialog::StyleConfigDialog(QWidget* parent, const QString &styleName)
  : QDialog( parent )
{
  setObjectName( QStringLiteral("StyleConfigDialog") );
  setModal( true );
  setWindowTitle( i18n( "Configure %1", styleName ) );
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QWidget *mainWidget = new QWidget(this);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults, this);
  mainLayout->addWidget(mainWidget);

  mMainLayout = new QHBoxLayout(mainWidget);
  mMainLayout->setContentsMargins(0, 0, 0, 0);

  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &StyleConfigDialog::slotAccept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &StyleConfigDialog::defaults);
  mainLayout->addWidget(buttonBox);

  buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
  m_dirty = false;
}

bool StyleConfigDialog::isDirty() const
{
  return m_dirty;
}

void StyleConfigDialog::setDirty(bool dirty)
{
  m_dirty = dirty;
}

void StyleConfigDialog::slotAccept()
{
    Q_EMIT save();
    QDialog::accept();
}

void StyleConfigDialog::setMainWidget(QWidget *w)
{
    mMainLayout->addWidget(w);
}
