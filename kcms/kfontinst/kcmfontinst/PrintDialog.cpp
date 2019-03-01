/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "PrintDialog.h"
#include <QFrame>
#include <QLabel>
#include <QGridLayout>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace KFI
{

CPrintDialog::CPrintDialog(QWidget *parent)
            : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18n("Print Font Samples"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CPrintDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CPrintDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QFrame *page = new QFrame(this);
    QGridLayout *layout=new QGridLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *lbl=new QLabel(i18n("Select size to print font:"), page);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layout->addWidget(lbl, 0, 0);
    itsSize=new QComboBox(page);
    itsSize->insertItem(0, i18n("Waterfall"));
    itsSize->insertItem(1, i18n("12pt"));
    itsSize->insertItem(2, i18n("18pt"));
    itsSize->insertItem(3, i18n("24pt"));
    itsSize->insertItem(4, i18n("36pt"));
    itsSize->insertItem(5, i18n("48pt"));
    itsSize->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    layout->addWidget(itsSize, 0, 1);
    layout->addItem(new QSpacerItem(2, 2), 2, 0);

    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
}

bool CPrintDialog::exec(int size)
{
    itsSize->setCurrentIndex(size);
    return QDialog::Accepted==QDialog::exec();
}

}
