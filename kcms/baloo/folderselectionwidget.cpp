/*
 * This file is part of the KDE Baloo Project
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

#include "folderselectionwidget.h"

#include <Solid/Device>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>

#include <QIcon>
#include <QFileDialog>

#include <QDir>
#include <QTimer>
#include <QUrl>
#include <KLocalizedString>
#include <QSpacerItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

FolderSelectionWidget::FolderSelectionWidget(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);

    m_messageWidget = new KMessageWidget(this);
    m_messageWidget->hide();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_messageWidget);
    layout->addWidget(m_listWidget);

    QHBoxLayout* hLayout = new QHBoxLayout;
    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addItem(spacer);

    m_addButton = new QPushButton(this);
    m_addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    connect(m_addButton, &QPushButton::clicked, this, &FolderSelectionWidget::slotAddButtonClicked);

    m_removeButton = new QPushButton(this);
    m_removeButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked, this, &FolderSelectionWidget::slotRemoveButtonClicked);

    connect(m_listWidget, &QListWidget::currentItemChanged, this, &FolderSelectionWidget::slotCurrentItemChanged);

    hLayout->addWidget(m_addButton);
    hLayout->addWidget(m_removeButton);
    layout->addItem(hLayout);
}

namespace {
    QStringList addTrailingSlashes(const QStringList& input) {
        QStringList output;
        Q_FOREACH (QString str, input) {
            if (!str.endsWith(QDir::separator()))
                str.append(QDir::separator());

            output << str;
        }

        return output;
    }

    QString makeHomePretty(const QString& url) {
        if (url.startsWith(QDir::homePath()))
            return QString(url).replace(0, QDir::homePath().length(), QLatin1String("~"));
        return url;
    }
}
void FolderSelectionWidget::setFolders(QStringList includeDirs, QStringList exclude)
{
    m_listWidget->clear();
    m_mountPoints.clear();

    QList<Solid::Device> devices
        = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    Q_FOREACH (const Solid::Device& dev, devices) {
        const Solid::StorageAccess* sa = dev.as<Solid::StorageAccess>();
        if (!sa->isAccessible())
            continue;

        QString mountPath = sa->filePath();
        if (!shouldShowMountPoint(mountPath))
            continue;

        m_mountPoints << mountPath;
    }
    m_mountPoints << QDir::homePath();

    m_mountPoints = addTrailingSlashes(m_mountPoints);
    includeDirs = addTrailingSlashes(includeDirs);
    exclude = addTrailingSlashes(exclude);

    QStringList excludeList = exclude;
    Q_FOREACH (const QString& mountPath, m_mountPoints) {
        if (includeDirs.contains(mountPath))
            continue;

        if (exclude.contains(mountPath))
            continue;

        excludeList << mountPath;
    }

    Q_FOREACH (QString url, excludeList) {
        QListWidgetItem* item = new QListWidgetItem(m_listWidget);
        QString display = getFolderDisplayName(url);

        item->setData(Qt::DisplayRole, display);
        item->setData(Qt::WhatsThisRole, url);
        item->setData(UrlRole, url);
        item->setData(Qt::DecorationRole, QIcon::fromTheme(iconName(url)));
        item->setToolTip(makeHomePretty(url));

        m_listWidget->addItem(item);
    }

    if (m_listWidget->count() == 0) {
        m_removeButton->setEnabled(false);
    }
}

QStringList FolderSelectionWidget::includeFolders() const
{
    QStringList folders;
    Q_FOREACH (const QString& mountPath, m_mountPoints) {
        bool inExclude = false;
        for (int i=0; i<m_listWidget->count(); ++i) {
            QListWidgetItem* item = m_listWidget->item(i);
            QString url = item->data(UrlRole).toString();

            if (mountPath == url) {
                inExclude = true;
                break;
            }
        }

        if (!inExclude) {
            folders << mountPath;
        }
    }

    return folders;
}

QStringList FolderSelectionWidget::excludeFolders() const
{
    QStringList folders;
    for (int i=0; i<m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString url = item->data(UrlRole).toString();

        folders << url;
    }

    return folders;
}

QString FolderSelectionWidget::fetchMountPoint(const QString& url) const
{
    QString mountPoint;

    Q_FOREACH (const QString& mount, m_mountPoints) {
        if (url.startsWith(mount) && mount.size() > mountPoint.size())
            mountPoint = mount;
    }

    return mountPoint;
}


void FolderSelectionWidget::slotAddButtonClicked()
{
    QString url = QFileDialog::getExistingDirectory(this, i18n("Select the folder which should be excluded"));
    if (url.isEmpty()) {
        return;
    }

    if (!url.endsWith(QDir::separator()))
        url.append(QDir::separator());

    // We don't care about the root dir
    if (url == QLatin1String("/")) {
        showMessage(i18n("The root directory is always hidden"));
    }

    // Remove any existing folder with that name
    // Remove any folder which is a sub-folder
    QVector<QListWidgetItem*> deleteList;

    for (int i=0; i<m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString existingUrl = item->data(UrlRole).toString();

        if (existingUrl == url) {
            QString name = QUrl::fromLocalFile(url).fileName();
            showMessage(i18n("Folder %1 is already excluded").arg(name));

            deleteList << item;
            continue;
        }

        QString existingMountPoint = fetchMountPoint(existingUrl);
        QString mountPoint = fetchMountPoint(url);

        if (existingMountPoint == mountPoint) {
            // existingUrl is not required since it comes under url
            if (existingUrl.startsWith(url)) {
                deleteList << item;
            }
            else if (url.startsWith(existingUrl)) {
                // No point adding ourselves since our parents exists
                // we just move the parent to the bottom
                url = existingUrl;
                deleteList << item;

                QString name = QUrl::fromLocalFile(url).fileName();
                showMessage(i18n("Folder's parent %1 is already excluded").arg(name));
            }
        }
    }
    qDeleteAll(deleteList);

    QListWidgetItem* item = new QListWidgetItem(m_listWidget);
    QString display = getFolderDisplayName(url);

    item->setData(Qt::DisplayRole, display);
    item->setData(Qt::WhatsThisRole, url);
    item->setData(UrlRole, url);
    item->setData(Qt::DecorationRole, QIcon::fromTheme(iconName(url)));
    item->setToolTip(makeHomePretty(url));

    m_listWidget->addItem(item);
    m_listWidget->setCurrentItem(item);

    Q_EMIT changed();
}

void FolderSelectionWidget::slotRemoveButtonClicked()
{
    QListWidgetItem* item = m_listWidget->currentItem();
    delete item;

    Q_EMIT changed();
}

void FolderSelectionWidget::slotCurrentItemChanged(QListWidgetItem* current, QListWidgetItem*)
{
    m_removeButton->setEnabled(current != 0);
}

void FolderSelectionWidget::showMessage(const QString& message)
{
    m_messageWidget->setText(message);
    m_messageWidget->setMessageType(KMessageWidget::Warning);
    m_messageWidget->animatedShow();

    QTimer::singleShot(3000, m_messageWidget, SLOT(animatedHide()));
}


QString FolderSelectionWidget::getFolderDisplayName(const QString& url) const
{
    QString name = url;

    // Check Home Dir
    QString homePath = QDir::homePath() + QLatin1Char('/');
    if (url == homePath) {
        return QDir(homePath).dirName();
    }

    if (url.startsWith(homePath)) {
        name = url.mid(homePath.size());
    }
    else {
        // Check Mount allMountPointsExcluded
        Q_FOREACH (QString mountPoint, m_mountPoints) {
            if (url.startsWith(mountPoint)) {
                name = QLatin1Char('[') + QDir(mountPoint).dirName() + QLatin1String("]/") + url.mid(mountPoint.length());
                break;
            }
        }
    }

    if (name.endsWith(QLatin1Char('/'))) {
        name = name.mid(0, name.size() - 1);
    }
    return name;
}

bool FolderSelectionWidget::shouldShowMountPoint(const QString& mountPoint)
{
    if (mountPoint == QLatin1String("/"))
        return false;

    if (mountPoint.startsWith(QLatin1String("/boot")))
        return false;

    if (mountPoint.startsWith(QLatin1String("/tmp")))
        return false;

    // The user's home directory is forcibly added so we can ignore /home
    // if /home actually contains the home direcory
    if (mountPoint.startsWith(QStringLiteral("/home")) && QDir::homePath().startsWith(QStringLiteral("/home")))
        return false;

    return true;
}

bool FolderSelectionWidget::allMountPointsExcluded() const
{
    return excludeFolders().toSet() == m_mountPoints.toSet();
}

QString FolderSelectionWidget::iconName(QString path) const
{
    // Ensure paths end with /
    if (!path.endsWith(QDir::separator()))
        path.append(QDir::separator());

    QString homePath = QDir::homePath();
    if (!homePath.endsWith(QDir::separator()))
        homePath.append(QDir::separator());

    if (path == homePath)
        return QLatin1String("user-home");

    if (m_mountPoints.contains(path))
        return QLatin1String("drive-harddisk");

    return QLatin1String("folder");
}

