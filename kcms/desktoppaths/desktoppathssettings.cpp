/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "desktoppathssettings.h"

#include <QDir>

#include <KFilePlacesModel>
#include <KLocalizedString>

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
    Q_PROPERTY(QUrl publicLocation READ publicLocation WRITE setPublicLocation)
    Q_PROPERTY(QUrl templatesLocation READ templatesLocation WRITE setTemplatesLocation)
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

    QUrl publicLocation() const
    {
        return readUrl(QStringLiteral("XDG_PUBLICSHARE_DIR"), m_settings->defaultPublicLocation());
    }

    void setPublicLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_PUBLICSHARE_DIR"), url);
    }

    QUrl templatesLocation() const
    {
        return readUrl(QStringLiteral("XDG_TEMPLATES_DIR"), m_settings->defaultTemplatesLocation());
    }

    void setTemplatesLocation(const QUrl &url)
    {
        writeUrl(QStringLiteral("XDG_TEMPLATES_DIR"), url);
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
        const auto oldUrl = readUrl(key, QUrl());

        KConfigGroup group(m_settings->config(), QString());
        // HACK to benefit from path translation (thus unexpanding $HOME)
        group.writePathEntry(key, url.toLocalFile());
        const auto path = group.readEntryUntranslated(key, QString());
        group.writeEntry(key, QString(QStringLiteral("\"") + path + QStringLiteral("\"")));

        if (oldUrl.isValid() && oldUrl != url) {
            KFilePlacesModel placesModel;

            const auto &bookmark = placesModel.bookmarkForUrl(oldUrl);
            if (bookmark.isNull()) {
                return;
            }
            const auto &matchedPlaces = placesModel.match(placesModel.index(0, 0), KFilePlacesModel::UrlRole, oldUrl, 1, Qt::MatchFixedString);
            if (matchedPlaces.count() == 1) {
                placesModel.editPlace(matchedPlaces.at(0), url.fileName(), url, bookmark.icon(), bookmark.metaDataItem(QStringLiteral("OnlyInApp")));
            }
        }
    }

    DesktopPathsSettings *const m_settings;
};

DesktopPathsSettings::DesktopPathsSettings(QObject *parent)
    : KCoreConfigSkeleton(userDirsConfig(), parent)
    , m_xdgPathsStore(new XdgPathsSettingsStore(this))
{
    addItemInternal("desktopLocation", defaultDesktopLocation(), [this] {
        Q_EMIT desktopLocationChanged();
    });
    addItemInternal("documentsLocation", defaultDocumentsLocation(), [this] {
        Q_EMIT documentsLocationChanged();
    });
    addItemInternal("downloadsLocation", defaultDownloadsLocation(), [this] {
        Q_EMIT downloadsLocationChanged();
    });
    addItemInternal("musicLocation", defaultMusicLocation(), [this] {
        Q_EMIT musicLocationChanged();
    });
    addItemInternal("picturesLocation", defaultPicturesLocation(), [this] {
        Q_EMIT picturesLocationChanged();
    });
    addItemInternal("videosLocation", defaultVideosLocation(), [this] {
        Q_EMIT videosLocationChanged();
    });
    addItemInternal("publicLocation", defaultPublicLocation(), [this] {
        Q_EMIT publicLocationChanged();
    });
    addItemInternal("templatesLocation", defaultTemplatesLocation(), [this] {
        Q_EMIT templatesLocationChanged();
    });
}

void DesktopPathsSettings::addItemInternal(const QByteArray &propertyName, const QVariant &defaultValue, const std::function<void()> &signal)
{
    auto *item = new KPropertySkeletonItem(m_xdgPathsStore, propertyName, defaultValue);
    item->setNotifyFunction(signal);
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
    const char *desktop = "Desktop";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", desktop));
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
    const char *documents = "Documents";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", documents));
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
    const char *downloads = "Downloads";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", downloads));
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
    const char *music = "Music";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", music));
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
    const char *pictures = "Pictures";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", pictures));
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
    const char *videos = "Videos";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", videos));
}

QUrl DesktopPathsSettings::publicLocation() const
{
    return findItem("publicLocation")->property().toUrl();
}

void DesktopPathsSettings::setPublicLocation(const QUrl &url)
{
    findItem("publicLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultPublicLocation() const
{
    const char *pub = "Public";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", pub));
}

QUrl DesktopPathsSettings::templatesLocation() const
{
    return findItem("templatesLocation")->property().toUrl();
}

void DesktopPathsSettings::setTemplatesLocation(const QUrl &url)
{
    findItem("templatesLocation")->setProperty(url);
}

QUrl DesktopPathsSettings::defaultTemplatesLocation() const
{
    const char *templates = "Templates";
    return QUrl::fromLocalFile(QDir::homePath() + QDir::separator() + i18nd("xdg-user-dirs", templates));
}

#include "desktoppathssettings.moc"

#include "moc_desktoppathssettings.cpp"
