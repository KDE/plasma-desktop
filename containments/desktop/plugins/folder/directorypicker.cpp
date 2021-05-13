/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "directorypicker.h"

#include <QFileDialog>
#include <QStandardPaths>

#include <KLocalizedString>

DirectoryPicker::DirectoryPicker(QObject *parent)
    : QObject(parent)
    , m_dialog(nullptr)
{
}

DirectoryPicker::~DirectoryPicker()
{
    delete m_dialog;
}

QUrl DirectoryPicker::url() const
{
    return m_url;
}

void DirectoryPicker::open()
{
    if (!m_dialog) {
        m_dialog = new QFileDialog(nullptr, i18n("Select Folder"), QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0));
        m_dialog->setFileMode(QFileDialog::Directory);
        m_dialog->setOption(QFileDialog::ShowDirsOnly, true);
        connect(m_dialog, &QDialog::accepted, this, &DirectoryPicker::dialogAccepted);
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void DirectoryPicker::dialogAccepted()
{
    const QList<QUrl> &urls = m_dialog->selectedUrls();

    if (!urls.isEmpty()) {
        m_url = urls.at(0);

        Q_EMIT urlChanged();
    }
}
