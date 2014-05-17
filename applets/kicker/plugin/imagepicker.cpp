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

#include "imagepicker.h"

#include <QFileDialog>
#include <QStandardPaths>

#include <KLocalizedString>

ImagePicker::ImagePicker(QObject *parent) : QObject(parent),
    m_dialog(0)
{
}

ImagePicker::~ImagePicker()
{
    delete m_dialog;
}

QUrl ImagePicker::url() const
{
    return m_url;
}

void ImagePicker::open()
{
    if (!m_dialog) {
        QString path;
        const QStringList &locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

        if (!locations.isEmpty()) {
            path = locations.at(0);
        } else {
            // HomeLocation is guaranteed not to be empty.
            path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
        }

        m_dialog = new QFileDialog(0, i18n("Choose an image"),
                                      path,
                                      i18n("Image Files (*.png *.jpg *.jpeg *.bmp *.svg *.svgz)"));
        m_dialog->setFileMode(QFileDialog::ExistingFile);
        connect(m_dialog, &QDialog::accepted, this, &ImagePicker::dialogAccepted);
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void ImagePicker::dialogAccepted()
{
    const QList<QUrl> &urls = m_dialog->selectedUrls();

    if (!urls.isEmpty()) {
        m_url = urls.at(0);

        emit urlChanged();
    }
}
