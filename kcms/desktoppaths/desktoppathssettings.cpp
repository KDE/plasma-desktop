/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "desktoppathssettings.h"

#include <KLocalizedString>

#include <QDir>

namespace
{
// save in XDG user-dirs.dirs config file, this is where KGlobalSettings/QDesktopServices reads from.
KSharedConfig::Ptr userDirsConfig()
{
    const QString userDirsFilePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/user-dirs.dirs");
    return KSharedConfig::openConfig(userDirsFilePath, KConfig::SimpleConfig);
}
}

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
        return readUrl(QStringLiteral("XDG_DESKTOP_DIR"), m_settings->defaultDesktopLocation());
    }

    void setDesktopLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_DESKTOP_DIR"), url);
    }

    QUrl documentsLocation() const
    {
        return readUrl(QStringLiteral("XDG_DOCUMENTS_DIR"), m_settings->defaultDocumentsLocation());
    }

    void setDocumentsLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_DOCUMENTS_DIR"), url);
    }

    QUrl downloadsLocation() const
    {
        return readUrl(QStringLiteral("XDG_DOWNLOAD_DIR"), m_settings->defaultDownloadsLocation());
    }

    void setDownloadsLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_DOWNLOAD_DIR"), url);
    }

    QUrl musicLocation() const
    {
        return readUrl(QStringLiteral("XDG_MUSIC_DIR"), m_settings->defaultMusicLocation());
    }

    void setMusicLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_MUSIC_DIR"), url);
    }

    QUrl picturesLocation() const
    {
        return readUrl(QStringLiteral("XDG_PICTURES_DIR"), m_settings->defaultPicturesLocation());
    }

    void setPicturesLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_PICTURES_DIR"), url);
    }

    QUrl videosLocation() const
    {
        return readUrl(QStringLiteral("XDG_VIDEOS_DIR"), m_settings->defaultVideosLocation());
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
    , m_xdgPathsStore(new XdgPathsSettingsStore(this))
{
    addItemInternal("desktopLocation", defaultDesktopLocation());
    addItemInternal("documentsLocation", defaultDocumentsLocation());
    addItemInternal("downloadsLocation", defaultDownloadsLocation());
    addItemInternal("musicLocation", defaultMusicLocation());
    addItemInternal("picturesLocation", defaultPicturesLocation());
    addItemInternal("videosLocation", defaultVideosLocation());
}

void DesktopPathsSettings::addItemInternal(const QByteArray &propertyName, const QVariant &defaultValue)
{
    auto *item = new KPropertySkeletonItem(m_xdgPathsStore, propertyName, defaultValue);
    item->setNotifyFunction([this] {
        Q_EMIT this->widgetChanged();
    });
    addItem(item, propertyName);
}

QUrl DesktopPathsSettings::desktopLocation() const
{
    return findItem("desktopLocation")->property().toUrl();
}

void DesktopPathsSettings::setDesktopLocation(const QUrl &url)
{
    findItem("desktopLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultDesktopLocation() const
{
    return QUrl::fromLocalFile(QDir::homePath() + QLatin1Char('/') + i18nd("xdg-user-dirs", "Desktop"));
}

QUrl DesktopPathsSettings::documentsLocation() const
{
    return findItem("documentsLocation")->property().toUrl();
}

void DesktopPathsSettings::setDocumentsLocation(const QUrl &url)
{
    findItem("documentsLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultDocumentsLocation() const
{
    return QUrl::fromLocalFile(QDir::homePath() + QLatin1Char('/') + i18nd("xdg-user-dirs", "Documents"));
}

QUrl DesktopPathsSettings::downloadsLocation() const
{
    return findItem("downloadsLocation")->property().toUrl();
}

void DesktopPathsSettings::setDownloadsLocation(const QUrl &url)
{
    findItem("downloadsLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultDownloadsLocation() const
{
    return QUrl::fromLocalFile(QDir::homePath() + QLatin1Char('/') + i18nd("xdg-user-dirs", "Downloads"));
}

QUrl DesktopPathsSettings::musicLocation() const
{
    return findItem("musicLocation")->property().toUrl();
}

void DesktopPathsSettings::setMusicLocation(const QUrl &url)
{
    findItem("musicLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultMusicLocation() const
{
    return QUrl::fromLocalFile(QDir::homePath() + QLatin1Char('/') + i18nd("xdg-user-dirs", "Music"));
}

QUrl DesktopPathsSettings::picturesLocation() const
{
    return findItem("picturesLocation")->property().toUrl();
}

void DesktopPathsSettings::setPicturesLocation(const QUrl &url)
{
    findItem("picturesLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultPicturesLocation() const
{
    return QUrl::fromLocalFile(QDir::homePath() + QLatin1Char('/') + i18nd("xdg-user-dirs", "Pictures"));
}

QUrl DesktopPathsSettings::videosLocation() const
{
    return findItem("videosLocation")->property().toUrl();
}

void DesktopPathsSettings::setVideosLocation(const QUrl &url)
{
    findItem("videosLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultVideosLocation() const
{
    return QUrl::fromLocalFile(QDir::homePath() + QLatin1Char('/') + i18nd("xdg-user-dirs", "Videos"));
}

#include "desktoppathssettings.moc"
