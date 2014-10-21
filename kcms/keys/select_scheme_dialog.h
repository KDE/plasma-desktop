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
#ifndef SELECT_SCHEME_DIALOG_H
#define SELECT_SCHEME_DIALOG_H

#include "QDialog"
#include <QUrl>
#include <QPushButton>

namespace Ui
{
class SelectSchemeDialog;
}

class SelectSchemeDialog : public QDialog
{
    Q_OBJECT
public:
    SelectSchemeDialog(QWidget *parent = 0);
    ~SelectSchemeDialog();

    QUrl selectedScheme() const;

private Q_SLOTS:
    void schemeActivated(int index);
    void slotUrlChanged(const QString &);

private:
    Ui::SelectSchemeDialog *ui;
    QStringList m_schemes;
    QPushButton *mOkButton;
}; // SelectSchemeDialog



#endif
