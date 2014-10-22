/*
 * This file is part of the KDE Baloo project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef FOLDERSELECTIONWIDGET_H
#define FOLDERSELECTIONWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <KMessageWidget>

class FolderSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FolderSelectionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

    void setFolders(QStringList includeDirs, QStringList exclude);
    QStringList includeFolders() const;
    QStringList excludeFolders() const;

    enum Roles {
        UrlRole = Qt::UserRole + 1
    };

    bool allMountPointsExcluded() const;

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void slotAddButtonClicked();
    void slotRemoveButtonClicked();
    void slotCurrentItemChanged(QListWidgetItem* current, QListWidgetItem*);

private:
    QString getFolderDisplayName(const QString& url) const;
    bool shouldShowMountPoint(const QString& mountPoint);
    QString fetchMountPoint(const QString& url) const;
    void showMessage(const QString& message);

    QString iconName(QString path) const;

    QListWidget* m_listWidget;
    QStringList m_mountPoints;

    QPushButton* m_addButton;
    QPushButton* m_removeButton;

    KMessageWidget* m_messageWidget;
};

#endif // FOLDERSELECTIONWIDGET_H
