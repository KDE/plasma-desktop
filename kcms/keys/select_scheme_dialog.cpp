/*
 *  Copyright 2008 Michael Jansen <kde@michael-jansen.biz>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "select_scheme_dialog.h"
#include "ui_select_scheme_dialog.h"


#include "KDialog"
#include "KStandardDirs"
#include <KLineEdit>
#include <KConfig>
#include <KGlobal>

SelectSchemeDialog::SelectSchemeDialog(QWidget *parent)
 : KDialog(parent),
   ui(new Ui::SelectSchemeDialog)
{
    m_schemes = KGlobal::dirs()->findAllResources("data", "kcmkeys/*.kksrc");

    ui->setupUi(this);
    setMainWidget(ui->layoutWidget);

    foreach (const QString &res, m_schemes) {
        KConfig config(res, KConfig::SimpleConfig);
        KConfigGroup group(&config, "Settings");
        QString name = group.readEntry("Name");

        if (name.isEmpty()) {
            name = res;
        }
        ui->m_schemes->addItem(name);
    }

    ui->m_schemes->setCurrentIndex(-1);

    ui->m_url->setMode(KFile::LocalOnly | KFile::ExistingOnly);

    connect(ui->m_schemes, SIGNAL(activated(int)),
            this, SLOT(schemeActivated(int)));
    connect(ui->m_url->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotUrlChanged(QString)));
    enableButtonOk(false);
}


SelectSchemeDialog::~SelectSchemeDialog()
{
    delete ui;
}

void SelectSchemeDialog::schemeActivated(int index)
{
    ui->m_url->setUrl(QUrl(m_schemes[index]));
}


KUrl SelectSchemeDialog::selectedScheme() const
{
    return ui->m_url->url();
}

void SelectSchemeDialog::slotUrlChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

#include "moc_select_scheme_dialog.cpp"
