/**
 *  Copyright 2020 Kevin Ottens <kevin.ottens@enioka.com>
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

#include "desktoppathssettings.h"

#include <QDir>

namespace {
    //save in XDG user-dirs.dirs config file, this is where KGlobalSettings/QDesktopServices reads from.
    KSharedConfig::Ptr userDirsConfig()
    {
        const QString userDirsFilePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/user-dirs.dirs");
        return KSharedConfig::openConfig(userDirsFilePath, KConfig::SimpleConfig);
    }

    QUrl defaultAutostartLocation()
    {
        return QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/.config/autostart"));
    }

    QUrl defaultDesktopLocation()
    {
        return QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/Desktop"));
    }

    QUrl defaultDocumentsLocation()
    {
        return QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/Documents"));
    }

    QUrl defaultDownloadsLocation()
    {
        return QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/Downloads"));
    }

    QUrl defaultMusicLocation()
    {
        return QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/Music"));
    }

    QUrl defaultPicturesLocation()
    {
        return QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/Pictures"));
    }

    QUrl defaultVideosLocation()
    {
        return QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/Videos"));
    }
}

class PathsSettingsStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl autostartLocation READ autostartLocation WRITE setAutostartLocation)
public:
    PathsSettingsStore(QObject *parent = nullptr)
        : QObject(parent)
        , m_config(KSharedConfig::openConfig())
    {
    }

    QUrl autostartLocation() const
    {
        return readUrl(QStringLiteral("Autostart"), defaultAutostartLocation());
    }

    void setAutostartLocation(const QUrl &url)
    {
        if (url.matches(defaultAutostartLocation(), QUrl::StripTrailingSlash)) {
            resetUrl(QStringLiteral("Autostart"));
        } else {
            writeUrl(QStringLiteral("Autostart"), url);
        }
    }

    void save()
    {
        if (m_config->isDirty()) {
            m_config->sync();
        }
    }

private:
    QUrl readUrl(const QString &key, const QUrl &defaultValue) const
    {
        KConfigGroup group(m_config, QStringLiteral("Paths"));
        const auto path = group.readPathEntry(key, QString());
        if (path.isEmpty()) {
            return defaultValue;
        } else {
            return QUrl::fromLocalFile(path);
        }
    }

    void writeUrl(const QString &key, const QUrl &url)
    {
        KConfigGroup group(m_config, QStringLiteral("Paths"));
        group.writePathEntry(key, url.toLocalFile(), KConfig::Normal | KConfig::Global);
    }

    void resetUrl(const QString &key)
    {
        KConfigGroup group(m_config, QStringLiteral("Paths"));
        group.revertToDefault(key, KConfig::Normal | KConfig::Global);
    }

    KSharedConfig::Ptr m_config;
};

class XdgPathsSettingsStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl desktopLocation READ desktopLocation WRITE setDesktopLocation)
    Q_PROPERTY(QUrl documentsLocation READ documentsLocation WRITE setDocumentsLocation)
    Q_PROPERTY(QUrl downloadsLocation READ downloadsLocation WRITE setDownloadsLocation)
    Q_PROPERTY(QUrl musicLocation READ musicLocation WRITE setMusicLocation)
    Q_PROPERTY(QUrl picturesLocation READ picturesLocation WRITE setPicturesLocation)
    Q_PROPERTY(QUrl videosLocation READ videosLocation WRITE setVideosLocation)
public:
    XdgPathsSettingsStore(DesktopPathsSettings *parent = nullptr)
        : QObject(parent)
        , m_settings(parent)
    {
    }

    QUrl desktopLocation() const
    {
        return readUrl(QStringLiteral("XDG_DESKTOP_DIR"), defaultDesktopLocation());
    }

    void setDesktopLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_DESKTOP_DIR"), url);
    }

    QUrl documentsLocation() const
    {
        return readUrl(QStringLiteral("XDG_DOCUMENTS_DIR"), defaultDocumentsLocation());
    }

    void setDocumentsLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_DOCUMENTS_DIR"), url);
    }

    QUrl downloadsLocation() const
    {
        return readUrl(QStringLiteral("XDG_DOWNLOAD_DIR"), defaultDownloadsLocation());
    }

    void setDownloadsLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_DOWNLOAD_DIR"), url);
    }

    QUrl musicLocation() const
    {
        return readUrl(QStringLiteral("XDG_MUSIC_DIR"), defaultMusicLocation());
    }

    void setMusicLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_MUSIC_DIR"), url);
    }

    QUrl picturesLocation() const
    {
        return readUrl(QStringLiteral("XDG_PICTURES_DIR"), defaultPicturesLocation());
    }

    void setPicturesLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_PICTURES_DIR"), url);
    }

    QUrl videosLocation() const
    {
        return readUrl(QStringLiteral("XDG_VIDEOS_DIR"), defaultVideosLocation());
    }

    void setVideosLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_VIDEOS_DIR"), url);
    }

private:
    QUrl readUrl(const QString &key, const QUrl &defaultValue) const
    {
        KConfigGroup group(m_settings->config(), QString());
        const auto path = group.readPathEntry(key, QString());
        if (path.isEmpty()) {
            return defaultValue;
        } else {
            return QUrl::fromLocalFile(path.mid(1, path.length() - 2));
        }
    }

    void writeUrl(const QString &key, const QUrl &url)
    {
        KConfigGroup group(m_settings->config(), QString());
        // HACK to benefit from path translation (thus unexpanding $HOME)
        group.writePathEntry(key, url.toLocalFile());
        const auto path = group.readEntryUntranslated(key, QString());
        group.writeEntry(key, QString(QStringLiteral("\"") + path + QStringLiteral("\"")));
    }

    DesktopPathsSettings *m_settings;
};

DesktopPathsSettings::DesktopPathsSettings(QObject *parent)
    : KCoreConfigSkeleton(userDirsConfig(), parent)
    , m_pathsStore(new PathsSettingsStore(this))
    , m_xdgPathsStore(new XdgPathsSettingsStore(this))
{
    addItem(new KPropertySkeletonItem(m_xdgPathsStore, "desktopLocation", defaultDesktopLocation()), "desktopLocation");
    addItem(new KPropertySkeletonItem(m_pathsStore, "autostartLocation", defaultAutostartLocation()), "autostartLocation");
    addItem(new KPropertySkeletonItem(m_xdgPathsStore, "documentsLocation", defaultDocumentsLocation()), "documentsLocation");
    addItem(new KPropertySkeletonItem(m_xdgPathsStore, "downloadsLocation", defaultDownloadsLocation()), "downloadsLocation");
    addItem(new KPropertySkeletonItem(m_xdgPathsStore, "musicLocation", defaultMusicLocation()), "musicLocation");
    addItem(new KPropertySkeletonItem(m_xdgPathsStore, "picturesLocation", defaultPicturesLocation()), "picturesLocation");
    addItem(new KPropertySkeletonItem(m_xdgPathsStore, "videosLocation", defaultVideosLocation()), "videosLocation");
}

QUrl DesktopPathsSettings::autostartLocation() const
{
    return findItem("autostartLocation")->property().toUrl();
}

void DesktopPathsSettings::setAutostartLocation(const QUrl &url)
{
    findItem("autostartLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::desktopLocation() const
{
    return findItem("desktopLocation")->property().toUrl();
}

void DesktopPathsSettings::setDesktopLocation(const QUrl &url)
{
    findItem("desktopLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::documentsLocation() const
{
    return findItem("documentsLocation")->property().toUrl();
}

void DesktopPathsSettings::setDocumentsLocation(const QUrl &url)
{
    findItem("documentsLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::downloadsLocation() const
{
    return findItem("downloadsLocation")->property().toUrl();
}

void DesktopPathsSettings::setDownloadsLocation(const QUrl &url)
{
    findItem("downloadsLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::musicLocation() const
{
    return findItem("musicLocation")->property().toUrl();
}

void DesktopPathsSettings::setMusicLocation(const QUrl &url)
{
    findItem("musicLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::picturesLocation() const
{
    return findItem("picturesLocation")->property().toUrl();
}

void DesktopPathsSettings::setPicturesLocation(const QUrl &url)
{
    findItem("picturesLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::videosLocation() const
{
    return findItem("videosLocation")->property().toUrl();
}

void DesktopPathsSettings::setVideosLocation(const QUrl &url)
{
    findItem("videosLocation")->setProperty(url);
}

bool DesktopPathsSettings::usrSave()
{
    m_pathsStore->save();
    return KCoreConfigSkeleton::usrSave();
}

#include "desktoppathssettings.moc"
