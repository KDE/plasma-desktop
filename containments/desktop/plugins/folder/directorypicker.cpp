/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
