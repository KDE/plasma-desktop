/***************************************************************************
 *   Copyright (C) 2007 by Stephen Leaf                                    *
 *   smileaf@gmail.com                                                     *
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

#include "addscriptdialog.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileInfo>

#include <KLineEdit>
#include <KLocalizedString>
#include <KUrlRequester>
#include <KShell>
#include <KMessageBox>

AddScriptDialog::AddScriptDialog (QWidget* parent)
    : QDialog( parent )
{
    QVBoxLayout *lay= new QVBoxLayout;
    setLayout( lay );
    QLabel *lab = new QLabel( i18n( "Shell script path:" ), this );
    lay->addWidget( lab );
    m_url = new KUrlRequester( this );
    lay->addWidget( m_url );
    m_symlink = new QCheckBox( i18n( "Create as symlink" ), this ); //TODO fix text
    m_symlink->setChecked( true );
    lay->addWidget( m_symlink );
    connect( m_url->lineEdit(), SIGNAL(textChanged(QString)), SLOT(textChanged(QString)) );
    m_url->lineEdit()->setFocus();

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    lay->addWidget(m_buttons);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &AddScriptDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &AddScriptDialog::reject);
}

AddScriptDialog::~AddScriptDialog()
{
}

void AddScriptDialog::textChanged(const QString &text)
{
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void AddScriptDialog::accept()
{
    if ( doBasicSanityCheck() )
        QDialog::accept();
}

bool AddScriptDialog::doBasicSanityCheck()
{
    const QString& path = KShell::tildeExpand(m_url->text());

    QFileInfo file(path);

    if ( ! file.isAbsolute() ) {
        KMessageBox::sorry( 0, i18n("\"%1\" is not an absolute path.", path) );
        return false;
    } else if ( ! file.exists() ) {
        KMessageBox::sorry( 0, i18n("\"%1\" does not exist.", path) );
        return false;
    } else if ( !file.isFile() ) {
        KMessageBox::sorry( 0, i18n("\"%1\" is not a file.", path) );
        return false;
    } else if ( ! file.isReadable() ) {
        KMessageBox::sorry( 0, i18n("\"%1\" is not readable.", path) );
        return false;
    }

    return true;
}

QUrl AddScriptDialog::importUrl() const
{
    return m_url->url();
}

bool AddScriptDialog::symLink() const
{
    return m_symlink->isChecked();
}
