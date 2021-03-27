/*
 *   Copyright 2012 by Marco Martin <mart@kde.org>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DIRMODEL_H
#define DIRMODEL_H

#include <KDirModel>
#include <KSharedDataCache>
#include <QSize>
#include <QVariant>
#include <kimagecache.h>

class QTimer;

/**
 * This class provides a QML binding to KDirModel
 * Provides an easy way to navigate a filesystem from within QML
 *
 * @author Marco Martin <mart@kde.org>
 */
class DirModel : public KDirModel
{
    Q_OBJECT

    /**
     * @property string The url we want to browse. it may be an absolute path or a correct url of any protocol KIO supports
     */
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

    /**
     * @property count Total number of rows
     */
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        MimeTypeRole = Qt::UserRole + 2,
        Thumbnail = Qt::UserRole + 3,
    };

    explicit DirModel(QObject *parent = nullptr);
    ~DirModel() override;

    QHash<int, QByteArray> roleNames() const override;

    void setUrl(const QString &url);
    QString url() const;

    QVariant data(const QModelIndex &index, int role) const override;
    int count() const
    {
        return rowCount();
    }

    Q_INVOKABLE int indexForUrl(const QString &url) const;

    Q_INVOKABLE QVariantMap get(int index) const;

    /**
     * Helper method to empty the trash
     */
    Q_INVOKABLE void emptyTrash();

protected Q_SLOTS:
    void showPreview(const KFileItem &item, const QPixmap &preview);
    void previewFailed(const KFileItem &item);
    void delayedPreview();

Q_SIGNALS:
    void countChanged();
    void urlChanged();

private:
    QStringList m_mimeTypes;

    // previews
    QTimer *m_previewTimer = nullptr;
    QHash<QUrl, QPersistentModelIndex> m_filesToPreview;
    QSize m_screenshotSize;
    QHash<QUrl, QPersistentModelIndex> m_previewJobs;
    KImageCache *m_imageCache = nullptr;
};

#endif // DIRMODEL_H
