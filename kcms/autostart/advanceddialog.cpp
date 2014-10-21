/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "advanceddialog.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <KLocalizedString>

AdvancedDialog::AdvancedDialog( QWidget *parent, bool status )
    : QDialog( parent )
{
    QVBoxLayout *lay= new QVBoxLayout(this);
    m_onlyInKde = new QCheckBox( i18n( "Autostart only in KDE" ), this );
    m_onlyInKde->setChecked( status );
    lay->addWidget( m_onlyInKde );

    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->setStandardButtons( QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lay->addWidget(buttons);
    setLayout( lay );

    connect(buttons, &QDialogButtonBox::accepted, this, &AdvancedDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &AdvancedDialog::reject);
}

AdvancedDialog::~AdvancedDialog()
{
}

bool AdvancedDialog::onlyInKde() const
{
    return m_onlyInKde->isChecked();
}
