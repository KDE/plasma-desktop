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

#ifndef ADDSCRIPTDIALOG_H
#define ADDSCRIPTDIALOG_H

#include <QDialog>
#include <QUrl>

class QDialogButtonBox;
class KUrlRequester;
class QCheckBox;

class AddScriptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddScriptDialog(QWidget* parent=0);
    ~AddScriptDialog();
    // Returns the Url of the script to be imported
    QUrl importUrl() const;
    bool symLink() const;

public Q_SLOTS:
    // reimplemented
    virtual void accept();

protected:
    virtual bool doBasicSanityCheck();

private Q_SLOTS:
    void textChanged(const QString &text);

private:
    KUrlRequester *m_url;
    QCheckBox* m_symlink;

    QDialogButtonBox* m_buttons;
};

#endif

