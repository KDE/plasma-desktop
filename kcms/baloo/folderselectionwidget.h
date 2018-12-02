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
    explicit FolderSelectionWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void setDirectoryList(QStringList includeDirs, QStringList exclude);
    QStringList includeFolders() const;
    QStringList excludeFolders() const;

    enum Roles {
        UrlRole = Qt::UserRole + 1
    };

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void slotAddButtonClicked();
    void slotRemoveButtonClicked();
    void slotCurrentItemChanged(QListWidgetItem* current, QListWidgetItem*);

private:
    QString folderDisplayName(const QString& url) const;
    bool shouldShowMountPoint(const QString& mountPoint);
    QString fetchMountPoint(const QString& url) const;
    void showMessage(const QString& message);

    /**
     * @brief Get the theme valid icon name for \p path.
     *
     * @param path Path to be analysed.
     * @return One of: "user-home", "drive-harddisk" or "folder"
     */
    QString iconName(QString path) const;

    /**
     * @brief Widget with the list of directories.
     *
     */
    QListWidget* m_listWidget;
    QStringList m_mountPoints;

    /**
     * @brief Button to add a directory to the list.
     *
     */
    QPushButton* m_addButton;
    /**
     * @brief Button to remove the selected directory from the list.
     *
     */
    QPushButton* m_removeButton;

    /**
     * @brief Information, warning or error message widget.
     *
     */
    KMessageWidget* m_messageWidget;
};

#endif // FOLDERSELECTIONWIDGET_H
