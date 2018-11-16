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

#include "menuentryeditor.h"

#include <QDir>
#include <QPointer>
#include <QStandardPaths>

#include <KPropertiesDialog>

MenuEntryEditor::MenuEntryEditor(QObject* parent) : QObject(parent)
{
}

MenuEntryEditor::~MenuEntryEditor()
{
}

bool MenuEntryEditor::canEdit(const QString& entryPath) const
{
    KFileItemList itemList;
    itemList << KFileItem(QUrl::fromLocalFile(entryPath));

    return KPropertiesDialog::canDisplay(itemList);
}

void MenuEntryEditor::edit(const QString& entryPath, const QString& menuId)
{
    const QString &appsPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    const QUrl &entryUrl = QUrl::fromLocalFile(entryPath);

    if (!appsPath.isEmpty() && entryUrl.isValid()) {
        const QDir appsDir(appsPath);
        const QString &fileName = entryUrl.fileName();

        if (appsDir.exists(fileName)) {
            KPropertiesDialog::showDialog(entryUrl, nullptr, false);
        } else {
            if (!appsDir.exists()) {
                if (!QDir::root().mkpath(appsPath)) {
                    return;
                }
            }

            KPropertiesDialog *dialog = new KPropertiesDialog(entryUrl,
                QUrl::fromLocalFile(appsPath), menuId);
            //KPropertiesDialog deletes itself
            dialog->show();
        }
    }
}
