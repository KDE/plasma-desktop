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


#include "QDialog"
#include <KLineEdit>
#include <KConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QDebug>

SelectSchemeDialog::SelectSchemeDialog(QWidget *parent)
 : QDialog(parent),
   ui(new Ui::SelectSchemeDialog)
{
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                            QStringLiteral("kcmkeys"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.kksrc"));
        for (const QString &file : fileNames) {
            if (m_schemes.contains(file)) {
                continue;
            }
            m_schemes.append(dir + QLatin1Char('/') + file);
        }
    }
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SelectSchemeDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SelectSchemeDialog::reject);

    ui->setupUi(this);
    mainLayout->addWidget(ui->layoutWidget);
    mainLayout->addWidget(buttonBox);

    for (const QString &res : qAsConst(m_schemes)) {
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
    connect(ui->m_url->lineEdit(), &QLineEdit::textChanged,
            this, &SelectSchemeDialog::slotUrlChanged);
    mOkButton->setEnabled(false);
}


SelectSchemeDialog::~SelectSchemeDialog()
{
    delete ui;
}

void SelectSchemeDialog::schemeActivated(int index)
{
    ui->m_url->setUrl(QUrl(m_schemes[index]));
}


QUrl SelectSchemeDialog::selectedScheme() const
{
    return ui->m_url->url();
}

void SelectSchemeDialog::slotUrlChanged(const QString &_text)
{
    mOkButton->setEnabled(!_text.isEmpty());
}

#include "moc_select_scheme_dialog.cpp"
